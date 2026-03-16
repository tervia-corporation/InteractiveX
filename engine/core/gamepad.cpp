#include "gamepad.h"
#include <windows.h>
#include <cstring>
#include <cmath>
#include <cstdio>

// ---------------------------------------------------------------------------
//  XInput — SAL macros isoladas (DX SDK June 2010 + GCC/MinGW)
// ---------------------------------------------------------------------------
#define __in
#define __out
#define __inout
#define __in_opt
#define __out_opt
#define __reserved
#define __in_bcount(x)
#define __out_bcount(x)
#define __in_bcount_opt(x)
#define __out_bcount_opt(x)
#include <xinput.h>
#undef __in
#undef __out
#undef __inout
#undef __in_opt
#undef __out_opt
#undef __reserved
#undef __in_bcount
#undef __out_bcount
#undef __in_bcount_opt
#undef __out_bcount_opt

// ---------------------------------------------------------------------------
//  DirectInput
// ---------------------------------------------------------------------------
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

// Linkar: xinput  dinput8  dxguid
// (no GCC adicionar manualmente em Project -> Build Options -> Linker Settings)

// ---------------------------------------------------------------------------
//  GUIDs dos controles conhecidos para mapeamento correto
//  (VID/PID embutidos no GUID do produto do DirectInput)
// ---------------------------------------------------------------------------
// Sony DualShock 4 (varios revisoes)
static const DWORD DS4_VIDS[]  = { 0x054C, 0x054C, 0x054C };
static const DWORD DS4_PIDS[]  = { 0x05C4, 0x09CC, 0x0BA0 };  // v1, v2, USB wireless

// Sony DualSense (PS5)
static const DWORD DSS_VID  = 0x054C;
static const DWORD DSS_PID  = 0x0CE6;

// Nintendo Switch Pro Controller
static const DWORD NSW_VID  = 0x057E;
static const DWORD NSW_PID  = 0x2009;

// ---------------------------------------------------------------------------
//  Mapeamento DInput — indices de botao raw -> flags IX_GAMEPAD_*
//  Cada layout cobre um perfil de controle diferente
// ---------------------------------------------------------------------------
enum DInputLayout { DI_LAYOUT_GENERIC, DI_LAYOUT_DS4, DI_LAYOUT_DUALSENSE, DI_LAYOUT_SWITCH };

// DS4 e DualSense: botoes raw 0-13 em ordem Sony
static const unsigned int DS4_BUTTON_MAP[14] = {
    IX_GAMEPAD_X,      // 0  Quadrado
    IX_GAMEPAD_A,      // 1  Cruz
    IX_GAMEPAD_B,      // 2  Circulo
    IX_GAMEPAD_Y,      // 3  Triangulo
    IX_GAMEPAD_LB,     // 4  L1
    IX_GAMEPAD_RB,     // 5  R1
    0,                 // 6  L2 (eixo, nao botao)
    0,                 // 7  R2 (eixo, nao botao)
    IX_GAMEPAD_BACK,   // 8  Share / Create
    IX_GAMEPAD_START,  // 9  Options
    IX_GAMEPAD_LTHUMB, // 10 L3
    IX_GAMEPAD_RTHUMB, // 11 R3
    0,                 // 12 PS (ignorado)
    0,                 // 13 Touchpad (ignorado)
};

// Switch Pro: botoes raw 0-13
static const unsigned int NSW_BUTTON_MAP[14] = {
    IX_GAMEPAD_B,      // 0  B
    IX_GAMEPAD_A,      // 1  A
    IX_GAMEPAD_X,      // 2  X (posicao invertida Nintendo)
    IX_GAMEPAD_Y,      // 3  Y
    IX_GAMEPAD_LB,     // 4  L
    IX_GAMEPAD_RB,     // 5  R
    0,                 // 6  ZL (eixo)
    0,                 // 7  ZR (eixo)
    IX_GAMEPAD_BACK,   // 8  Minus
    IX_GAMEPAD_START,  // 9  Plus
    IX_GAMEPAD_LTHUMB, // 10 L Stick
    IX_GAMEPAD_RTHUMB, // 11 R Stick
    0,                 // 12 Home (ignorado)
    0,                 // 13 Capture (ignorado)
};

// Generico: assume mapeamento XInput-like
static const unsigned int GENERIC_BUTTON_MAP[14] = {
    IX_GAMEPAD_A, IX_GAMEPAD_B, IX_GAMEPAD_X, IX_GAMEPAD_Y,
    IX_GAMEPAD_LB, IX_GAMEPAD_RB, 0, 0,
    IX_GAMEPAD_BACK, IX_GAMEPAD_START,
    IX_GAMEPAD_LTHUMB, IX_GAMEPAD_RTHUMB,
    0, 0
};

// ---------------------------------------------------------------------------
//  Estado interno
// ---------------------------------------------------------------------------
struct GamepadState
{
    int          connected;
    int          backend;        // IX_GAMEPAD_BACKEND_*
    DInputLayout diLayout;
    float        deadZone;
    char         name[64];

    unsigned int buttonsDown;
    unsigned int buttonsPrev;
    unsigned int buttonsPressed;
    unsigned int buttonsReleased;

    float axes[IX_AXIS_COUNT];
    float axesPrev[IX_AXIS_COUNT];

    int triggerDown[2];
    int triggerPrev[2];
    int triggerPressed[2];
    int triggerReleased[2];

    // XInput
    int xinputIndex;   // -1 se nao e XInput

    // DirectInput
    IDirectInputDevice8A* diDevice;
    GUID                  diGuid;
    int                   diXInputIndex; // indice XInput correspondente (-1 se nao existe)
};

static GamepadState g_pads[IX_GAMEPAD_MAX];

// DirectInput global
static IDirectInput8A* g_dinput = NULL;

// ---------------------------------------------------------------------------
//  Helpers gerais
// ---------------------------------------------------------------------------
static float ApplyDeadZone(float v, float dz)
{
    if (v > -dz && v < dz) return 0.0f;
    float sign = (v >= 0.0f) ? 1.0f : -1.0f;
    return sign * ((v * sign - dz) / (1.0f - dz));
}

static void UpdateTriggerButtons(GamepadState* pad)
{
    const float THRESH = 0.5f;
    for (int t = 0; t < 2; t++)
    {
        int axis   = (t == 0) ? IX_AXIS_LT : IX_AXIS_RT;
        int isDown = (pad->axes[axis] >= THRESH) ? 1 : 0;
        pad->triggerPressed [t] = (!pad->triggerPrev[t] &&  isDown) ? 1 : 0;
        pad->triggerReleased[t] = ( pad->triggerPrev[t] && !isDown) ? 1 : 0;
        pad->triggerDown    [t] = isDown;
    }
}

static void SavePrevState(GamepadState* pad)
{
    pad->buttonsPrev = pad->buttonsDown;
    std::memcpy(pad->axesPrev,    pad->axes,        sizeof(pad->axes));
    std::memcpy(pad->triggerPrev, pad->triggerDown, sizeof(pad->triggerDown));
}

// ---------------------------------------------------------------------------
//  Verifica se um VID/PID corresponde a um controle XInput
//  (para evitar processar o mesmo controle duas vezes)
// ---------------------------------------------------------------------------
static int IsXInputDevice(DWORD vid, DWORD pid)
{
    (void)vid; (void)pid;
    // Heuristica: Microsoft VID = 0x045E
    // Logitech F310/F710 em modo XInput = 0x046D
    // Qualquer controle "Xbox compatible" tipicamente tem VID 045E
    // Nao ha forma 100% confiavel de detectar sem chamar XInput
    // e verificar se ele responde — usamos lista branca conservadora.
    static const DWORD XINPUT_VIDS[] = { 0x045E, 0x0738, 0x0E6F, 0x0F0D, 0x1532, 0x24C6 };
    for (int i = 0; i < 6; i++)
        if (vid == XINPUT_VIDS[i]) return 1;
    return 0;
}

static DInputLayout DetectLayout(DWORD vid, DWORD pid, char* nameOut, int nameSize)
{
    // Sony DS4
    for (int i = 0; i < 3; i++)
        if (vid == DS4_VIDS[i] && pid == DS4_PIDS[i])
        {
            std::snprintf(nameOut, nameSize, "DualShock 4");
            return DI_LAYOUT_DS4;
        }
    // Sony DualSense
    if (vid == DSS_VID && pid == DSS_PID)
    {
        std::snprintf(nameOut, nameSize, "DualSense (PS5)");
        return DI_LAYOUT_DUALSENSE;
    }
    // Nintendo Switch Pro
    if (vid == NSW_VID && pid == NSW_PID)
    {
        std::snprintf(nameOut, nameSize, "Switch Pro Controller");
        return DI_LAYOUT_SWITCH;
    }
    std::snprintf(nameOut, nameSize, "Generic Controller");
    return DI_LAYOUT_GENERIC;
}

// ---------------------------------------------------------------------------
//  DInput: converte POV (DPad) para flags IX_GAMEPAD_DPAD_*
// ---------------------------------------------------------------------------
static unsigned int POVToFlags(DWORD pov)
{
    if (LOWORD(pov) == 0xFFFF) return 0;  // centrado
    // pov em centesimos de grau, 0 = norte, sentido horario
    unsigned int flags = 0;
    if (pov >= 31500 || pov <= 4500)  flags |= IX_GAMEPAD_DPAD_UP;
    if (pov >= 4500  && pov <= 13500) flags |= IX_GAMEPAD_DPAD_RIGHT;
    if (pov >= 13500 && pov <= 22500) flags |= IX_GAMEPAD_DPAD_DOWN;
    if (pov >= 22500 && pov <= 31500) flags |= IX_GAMEPAD_DPAD_LEFT;
    return flags;
}

// ---------------------------------------------------------------------------
//  DInput: normaliza eixo raw (0..65535) para -1..1
// ---------------------------------------------------------------------------
static float NormalizeDIAxis(LONG raw)
{
    // DInput reporta 0..65535; centro = 32767
    float v = ((float)raw - 32767.0f) / 32768.0f;
    if (v >  1.0f) v =  1.0f;
    if (v < -1.0f) v = -1.0f;
    return v;
}

// ---------------------------------------------------------------------------
//  DInput: normaliza gatilho raw (0..65535) para 0..1
// ---------------------------------------------------------------------------
static float NormalizeDITrigger(LONG raw)
{
    return (float)raw / 65535.0f;
}

// ---------------------------------------------------------------------------
//  Callback de enumeracao DirectInput
// ---------------------------------------------------------------------------
struct DIEnumContext
{
    int slotStart;   // primeiro slot livre
};

static BOOL CALLBACK DIDeviceEnumCallback(LPCDIDEVICEINSTANCEA inst, LPVOID pvRef)
{
    DIEnumContext* ctx = (DIEnumContext*)pvRef;

    // Extrai VID/PID do GUID do produto
    DWORD vid = inst->guidProduct.Data1 & 0xFFFF;
    DWORD pid = (inst->guidProduct.Data1 >> 16) & 0xFFFF;

    // Pula controles XInput para nao duplicar
    if (IsXInputDevice(vid, pid)) return DIENUM_CONTINUE;

    // Procura slot livre que ainda nao tem DInput neste GUID
    for (int i = ctx->slotStart; i < IX_GAMEPAD_MAX; i++)
    {
        if (g_pads[i].connected && g_pads[i].backend == IX_GAMEPAD_BACKEND_DINPUT &&
            std::memcmp(&g_pads[i].diGuid, &inst->guidInstance, sizeof(GUID)) == 0)
            return DIENUM_CONTINUE;  // ja registrado
    }

    // Encontra slot livre
    int slot = -1;
    for (int i = 0; i < IX_GAMEPAD_MAX; i++)
    {
        if (!g_pads[i].connected) { slot = i; break; }
    }
    if (slot < 0) return DIENUM_STOP;  // sem slots

    // Cria dispositivo
    IDirectInputDevice8A* dev = NULL;
    if (FAILED(g_dinput->CreateDevice(inst->guidInstance, &dev, NULL)))
        return DIENUM_CONTINUE;

    if (FAILED(dev->SetDataFormat(&c_dfDIJoystick2)))
    {
        dev->Release();
        return DIENUM_CONTINUE;
    }

    // Sem janela associada — modo background permite leitura sem foco
    dev->SetCooperativeLevel(NULL, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
    dev->Acquire();

    // Configura range dos eixos para 0..65535
    DIPROPRANGE propRange;
    propRange.diph.dwSize       = sizeof(DIPROPRANGE);
    propRange.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    propRange.diph.dwHow        = DIPH_BYOFFSET;
    propRange.lMin              = 0;
    propRange.lMax              = 65535;

    DWORD axisOffsets[] = {
        DIJOFS_X, DIJOFS_Y, DIJOFS_Z, DIJOFS_RZ, DIJOFS_RX, DIJOFS_RY
    };
    for (int a = 0; a < 6; a++)
    {
        propRange.diph.dwObj = axisOffsets[a];
        dev->SetProperty(DIPROP_RANGE, &propRange.diph);
    }

    GamepadState* pad = &g_pads[slot];
    std::memset(pad, 0, sizeof(GamepadState));
    pad->connected    = 1;
    pad->backend      = IX_GAMEPAD_BACKEND_DINPUT;
    pad->deadZone     = 0.20f;
    pad->xinputIndex  = -1;
    pad->diDevice     = dev;
    pad->diGuid       = inst->guidInstance;
    pad->diLayout     = DetectLayout(vid, pid, pad->name, sizeof(pad->name));

    return DIENUM_CONTINUE;
}

// ---------------------------------------------------------------------------
//  XInput: atualiza um slot
// ---------------------------------------------------------------------------
static void UpdateXInput(int slot, int xinputIdx)
{
    GamepadState* pad = &g_pads[slot];

    SavePrevState(pad);

    XINPUT_STATE state;
    std::memset(&state, 0, sizeof(state));
    if (XInputGetState((DWORD)xinputIdx, &state) != ERROR_SUCCESS)
    {
        if (pad->connected)
        {
            XINPUT_VIBRATION vib = { 0, 0 };
            XInputSetState((DWORD)xinputIdx, &vib);
        }
        std::memset(pad, 0, sizeof(GamepadState));
        pad->deadZone    = 0.20f;
        pad->xinputIndex = -1;
        return;
    }

    if (!pad->connected)
    {
        pad->connected   = 1;
        pad->backend     = IX_GAMEPAD_BACKEND_XINPUT;
        pad->xinputIndex = xinputIdx;
        pad->deadZone    = 0.20f;
        std::snprintf(pad->name, sizeof(pad->name), "Xbox Controller");
    }

    pad->buttonsDown = state.Gamepad.wButtons;
    pad->buttonsPressed  = (~pad->buttonsPrev) & pad->buttonsDown;
    pad->buttonsReleased =  pad->buttonsPrev   & (~pad->buttonsDown);

    float dz = pad->deadZone;
    auto NA = [](SHORT r) -> float {
        float v = (r >= 0) ? (float)r / 32767.0f : (float)r / 32768.0f;
        return v > 1.0f ? 1.0f : (v < -1.0f ? -1.0f : v);
    };
    pad->axes[IX_AXIS_LX] = ApplyDeadZone( NA(state.Gamepad.sThumbLX), dz);
    pad->axes[IX_AXIS_LY] = ApplyDeadZone(-NA(state.Gamepad.sThumbLY), dz);
    pad->axes[IX_AXIS_RX] = ApplyDeadZone( NA(state.Gamepad.sThumbRX), dz);
    pad->axes[IX_AXIS_RY] = ApplyDeadZone(-NA(state.Gamepad.sThumbRY), dz);
    pad->axes[IX_AXIS_LT] = (float)state.Gamepad.bLeftTrigger  / 255.0f;
    pad->axes[IX_AXIS_RT] = (float)state.Gamepad.bRightTrigger / 255.0f;

    UpdateTriggerButtons(pad);
}

// ---------------------------------------------------------------------------
//  DirectInput: atualiza um slot
// ---------------------------------------------------------------------------
static void UpdateDInput(int slot)
{
    GamepadState* pad = &g_pads[slot];
    if (!pad->diDevice) return;

    SavePrevState(pad);

    if (FAILED(pad->diDevice->Poll()))
        pad->diDevice->Acquire();

    DIJOYSTATE2 js;
    if (FAILED(pad->diDevice->GetDeviceState(sizeof(DIJOYSTATE2), &js)))
    {
        // Dispositivo desconectado
        pad->diDevice->Release();
        std::memset(pad, 0, sizeof(GamepadState));
        pad->deadZone = 0.20f;
        return;
    }

    float dz = pad->deadZone;

    // Eixos — mapeamento difere por layout
    if (pad->diLayout == DI_LAYOUT_DS4 || pad->diLayout == DI_LAYOUT_DUALSENSE)
    {
        pad->axes[IX_AXIS_LX] = ApplyDeadZone( NormalizeDIAxis(js.lX),  dz);
        pad->axes[IX_AXIS_LY] = ApplyDeadZone( NormalizeDIAxis(js.lY),  dz);
        pad->axes[IX_AXIS_RX] = ApplyDeadZone( NormalizeDIAxis(js.lZ),  dz);
        pad->axes[IX_AXIS_RY] = ApplyDeadZone( NormalizeDIAxis(js.lRz), dz);
        pad->axes[IX_AXIS_LT] = NormalizeDITrigger(js.lRx);
        pad->axes[IX_AXIS_RT] = NormalizeDITrigger(js.lRy);
    }
    else if (pad->diLayout == DI_LAYOUT_SWITCH)
    {
        pad->axes[IX_AXIS_LX] = ApplyDeadZone( NormalizeDIAxis(js.lX),  dz);
        pad->axes[IX_AXIS_LY] = ApplyDeadZone( NormalizeDIAxis(js.lY),  dz);
        pad->axes[IX_AXIS_RX] = ApplyDeadZone( NormalizeDIAxis(js.lZ),  dz);
        pad->axes[IX_AXIS_RY] = ApplyDeadZone( NormalizeDIAxis(js.lRz), dz);
        pad->axes[IX_AXIS_LT] = NormalizeDITrigger(js.lRx);
        pad->axes[IX_AXIS_RT] = NormalizeDITrigger(js.lRy);
    }
    else
    {
        // Generico: XY = analogico esq, Z/Rz = analogico dir
        pad->axes[IX_AXIS_LX] = ApplyDeadZone( NormalizeDIAxis(js.lX),  dz);
        pad->axes[IX_AXIS_LY] = ApplyDeadZone( NormalizeDIAxis(js.lY),  dz);
        pad->axes[IX_AXIS_RX] = ApplyDeadZone( NormalizeDIAxis(js.lZ),  dz);
        pad->axes[IX_AXIS_RY] = ApplyDeadZone( NormalizeDIAxis(js.lRz), dz);
        pad->axes[IX_AXIS_LT] = 0.0f;
        pad->axes[IX_AXIS_RT] = 0.0f;
    }

    // Botoes — mapa pelo layout
    const unsigned int* btnMap = (pad->diLayout == DI_LAYOUT_DS4 ||
                                  pad->diLayout == DI_LAYOUT_DUALSENSE)
                                 ? DS4_BUTTON_MAP
                                 : (pad->diLayout == DI_LAYOUT_SWITCH)
                                   ? NSW_BUTTON_MAP
                                   : GENERIC_BUTTON_MAP;

    pad->buttonsDown = 0;
    int maxBtn = 14;
    for (int b = 0; b < maxBtn && b < 32; b++)
    {
        if ((js.rgbButtons[b] & 0x80) && btnMap[b])
            pad->buttonsDown |= btnMap[b];
    }

    // DPad via POV
    pad->buttonsDown |= POVToFlags(js.rgdwPOV[0]);

    pad->buttonsPressed  = (~pad->buttonsPrev) & pad->buttonsDown;
    pad->buttonsReleased =  pad->buttonsPrev   & (~pad->buttonsDown);

    UpdateTriggerButtons(pad);
}

// ---------------------------------------------------------------------------
//  Lifecycle
// ---------------------------------------------------------------------------
void Gamepad_Initialize()
{
    std::memset(g_pads, 0, sizeof(g_pads));
    for (int i = 0; i < IX_GAMEPAD_MAX; i++)
    {
        g_pads[i].deadZone    = 0.20f;
        g_pads[i].xinputIndex = -1;
    }

    // Inicializa DirectInput
    if (SUCCEEDED(DirectInput8Create(GetModuleHandleA(NULL),
        DIRECTINPUT_VERSION, IID_IDirectInput8A,
        (void**)&g_dinput, NULL)))
    {
        // Primeira enumeracao
        DIEnumContext ctx = { 0 };
        g_dinput->EnumDevices(DI8DEVCLASS_GAMECTRL, DIDeviceEnumCallback,
            &ctx, DIEDFL_ATTACHEDONLY);
    }
}

void Gamepad_Update()
{
    // --- Passo 1: atualiza slots XInput (indices 0-3 fixos) ---
    // Os 4 primeiros slots sao sempre reservados para XInput.
    // Se um slot XInput nao esta conectado, ele fica livre para DInput.
    for (int xi = 0; xi < IX_GAMEPAD_MAX; xi++)
    {
        // Encontra o slot que corresponde a este indice XInput
        int slot = -1;
        for (int s = 0; s < IX_GAMEPAD_MAX; s++)
        {
            if (g_pads[s].connected && g_pads[s].backend == IX_GAMEPAD_BACKEND_XINPUT
                && g_pads[s].xinputIndex == xi)
            {
                slot = s;
                break;
            }
        }

        XINPUT_STATE st;
        std::memset(&st, 0, sizeof(st));
        bool xiPresent = (XInputGetState((DWORD)xi, &st) == ERROR_SUCCESS);

        if (xiPresent)
        {
            if (slot < 0)
            {
                // Novo controle XInput — ocupa o primeiro slot livre
                for (int s = 0; s < IX_GAMEPAD_MAX; s++)
                {
                    if (!g_pads[s].connected)
                    {
                        slot = s;
                        std::memset(&g_pads[s], 0, sizeof(GamepadState));
                        g_pads[s].deadZone    = 0.20f;
                        g_pads[s].xinputIndex = xi;
                        break;
                    }
                }
            }
            if (slot >= 0) UpdateXInput(slot, xi);
        }
        else if (slot >= 0)
        {
            // Desconectou
            XINPUT_VIBRATION vib = { 0, 0 };
            XInputSetState((DWORD)xi, &vib);
            g_pads[slot].connected    = 0;
            g_pads[slot].backend      = IX_GAMEPAD_BACKEND_NONE;
            g_pads[slot].xinputIndex  = -1;
        }
    }

    // --- Passo 2: atualiza slots DInput existentes ---
    for (int s = 0; s < IX_GAMEPAD_MAX; s++)
    {
        if (g_pads[s].connected && g_pads[s].backend == IX_GAMEPAD_BACKEND_DINPUT)
            UpdateDInput(s);
    }

    // --- Passo 3: re-enumera periodicamente para detectar novos DInput ---
    static unsigned int g_enumFrame = 0;
    g_enumFrame++;
    if (g_enumFrame % 180 == 0 && g_dinput)  // a cada ~3s em 60fps
    {
        DIEnumContext ctx = { 0 };
        g_dinput->EnumDevices(DI8DEVCLASS_GAMECTRL, DIDeviceEnumCallback,
            &ctx, DIEDFL_ATTACHEDONLY);
    }
}

void Gamepad_Shutdown()
{
    for (int i = 0; i < IX_GAMEPAD_MAX; i++)
    {
        if (g_pads[i].backend == IX_GAMEPAD_BACKEND_XINPUT)
        {
            XINPUT_VIBRATION vib = { 0, 0 };
            XInputSetState((DWORD)g_pads[i].xinputIndex, &vib);
        }
        else if (g_pads[i].backend == IX_GAMEPAD_BACKEND_DINPUT && g_pads[i].diDevice)
        {
            g_pads[i].diDevice->Unacquire();
            g_pads[i].diDevice->Release();
        }
    }
    if (g_dinput) { g_dinput->Release(); g_dinput = NULL; }
    std::memset(g_pads, 0, sizeof(g_pads));
}

// ---------------------------------------------------------------------------
//  Queries
// ---------------------------------------------------------------------------
static int VI(int i) { return i >= 0 && i < IX_GAMEPAD_MAX; }

int   Gamepad_IsConnected(int i)                   { return VI(i) ? g_pads[i].connected : 0; }
int   Gamepad_GetBackend(int i)                    { return VI(i) ? g_pads[i].backend   : IX_GAMEPAD_BACKEND_NONE; }
const char* Gamepad_GetName(int i)                 { return (VI(i) && g_pads[i].connected) ? g_pads[i].name : "None"; }

int Gamepad_IsButtonDown(int i, unsigned int btn)
{
    return (VI(i) && g_pads[i].connected) ? ((g_pads[i].buttonsDown & btn) != 0) : 0;
}
int Gamepad_WasButtonPressed(int i, unsigned int btn)
{
    return (VI(i) && g_pads[i].connected) ? ((g_pads[i].buttonsPressed & btn) != 0) : 0;
}
int Gamepad_WasButtonReleased(int i, unsigned int btn)
{
    return (VI(i) && g_pads[i].connected) ? ((g_pads[i].buttonsReleased & btn) != 0) : 0;
}
float Gamepad_GetAxis(int i, int axis)
{
    if (!VI(i) || !g_pads[i].connected || axis < 0 || axis >= IX_AXIS_COUNT) return 0.0f;
    return g_pads[i].axes[axis];
}
int Gamepad_IsTriggerDown(int i, int axis)
{
    if (!VI(i) || !g_pads[i].connected) return 0;
    return g_pads[i].triggerDown[(axis == IX_AXIS_LT) ? 0 : 1];
}
int Gamepad_WasTriggerPressed(int i, int axis)
{
    if (!VI(i) || !g_pads[i].connected) return 0;
    return g_pads[i].triggerPressed[(axis == IX_AXIS_LT) ? 0 : 1];
}
int Gamepad_WasTriggerReleased(int i, int axis)
{
    if (!VI(i) || !g_pads[i].connected) return 0;
    return g_pads[i].triggerReleased[(axis == IX_AXIS_LT) ? 0 : 1];
}

// ---------------------------------------------------------------------------
//  Vibracao
// ---------------------------------------------------------------------------
void Gamepad_SetVibration(int i, float left, float right)
{
    if (!VI(i) || !g_pads[i].connected) return;
    if (g_pads[i].backend != IX_GAMEPAD_BACKEND_XINPUT) return; // DInput nao suporta
    if (left  < 0.0f) left  = 0.0f; if (left  > 1.0f) left  = 1.0f;
    if (right < 0.0f) right = 0.0f; if (right > 1.0f) right = 1.0f;
    XINPUT_VIBRATION vib;
    vib.wLeftMotorSpeed  = (WORD)(left  * 65535.0f);
    vib.wRightMotorSpeed = (WORD)(right * 65535.0f);
    XInputSetState((DWORD)g_pads[i].xinputIndex, &vib);
}
void Gamepad_StopVibration(int i)
{
    Gamepad_SetVibration(i, 0.0f, 0.0f);
}

// ---------------------------------------------------------------------------
//  Configuracao
// ---------------------------------------------------------------------------
void Gamepad_SetDeadZone(int i, float dz)
{
    if (!VI(i)) return;
    if (dz < 0.0f) dz = 0.0f;
    if (dz > 0.9f) dz = 0.9f;
    g_pads[i].deadZone = dz;
}
float Gamepad_GetDeadZone(int i) { return VI(i) ? g_pads[i].deadZone : 0.20f; }

int Gamepad_GetConnectedCount()
{
    int c = 0;
    for (int i = 0; i < IX_GAMEPAD_MAX; i++) if (g_pads[i].connected) c++;
    return c;
}

// ---------------------------------------------------------------------------
//  Utilitarios
// ---------------------------------------------------------------------------
const char* Gamepad_ButtonToString(unsigned int btn)
{
    switch (btn) {
        case IX_GAMEPAD_A:          return "A / Cross";
        case IX_GAMEPAD_B:          return "B / Circle";
        case IX_GAMEPAD_X:          return "X / Square";
        case IX_GAMEPAD_Y:          return "Y / Triangle";
        case IX_GAMEPAD_LB:         return "LB / L1";
        case IX_GAMEPAD_RB:         return "RB / R1";
        case IX_GAMEPAD_LTHUMB:     return "L3";
        case IX_GAMEPAD_RTHUMB:     return "R3";
        case IX_GAMEPAD_START:      return "Start / Options";
        case IX_GAMEPAD_BACK:       return "Back / Share";
        case IX_GAMEPAD_DPAD_UP:    return "DPad Up";
        case IX_GAMEPAD_DPAD_DOWN:  return "DPad Down";
        case IX_GAMEPAD_DPAD_LEFT:  return "DPad Left";
        case IX_GAMEPAD_DPAD_RIGHT: return "DPad Right";
        default:                    return "?";
    }
}
const char* Gamepad_AxisToString(int axis)
{
    switch (axis) {
        case IX_AXIS_LX: return "LX"; case IX_AXIS_LY: return "LY";
        case IX_AXIS_RX: return "RX"; case IX_AXIS_RY: return "RY";
        case IX_AXIS_LT: return "LT"; case IX_AXIS_RT: return "RT";
        default: return "?";
    }
}
