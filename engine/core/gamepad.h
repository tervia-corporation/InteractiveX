#ifndef IX_CORE_GAMEPAD_H
#define IX_CORE_GAMEPAD_H

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------
//  InteractiveX — Gamepad (XInput + DirectInput)
//
//  Suporta ate 4 controles simultaneos (indices 0-3).
//  Backend automatico: XInput para controles Xbox/genericos,
//  DirectInput para DualShock 4, DualSense, Switch Pro, etc.
//
//  Linkar nas configuracoes do projeto (Linker Settings):
//    xinput
//    dinput8
//    dxguid
//
//  API identica para ambos os backends — o jogo nao precisa saber qual
//  protocolo esta sendo usado.
//
//  Mapeamento DirectInput (DS4 / DualSense):
//    Cross   -> IX_GAMEPAD_A      Circulo  -> IX_GAMEPAD_B
//    Quadrado-> IX_GAMEPAD_X      Triangulo-> IX_GAMEPAD_Y
//    L1      -> IX_GAMEPAD_LB     R1       -> IX_GAMEPAD_RB
//    L2      -> IX_AXIS_LT        R2       -> IX_AXIS_RT
//    Share   -> IX_GAMEPAD_BACK   Options  -> IX_GAMEPAD_START
//    L3      -> IX_GAMEPAD_LTHUMB R3       -> IX_GAMEPAD_RTHUMB
//    DPad    -> IX_GAMEPAD_DPAD_*
//
//  Uso tipico:
//      Gamepad_Update();   // ja chamado por Engine_RunFrame
//
//      if (Gamepad_IsConnected(0)) {
//          if (Gamepad_WasButtonPressed(0, IX_GAMEPAD_A)) Jump();
//          float lx = Gamepad_GetAxis(0, IX_AXIS_LX);
//          player.vx = lx * SPEED;
//          Gamepad_SetVibration(0, 0.5f, 1.0f);
//      }
// ---------------------------------------------------------------------------

#define IX_GAMEPAD_MAX  4

// Bitmasks de botoes
#define IX_GAMEPAD_DPAD_UP        0x0001
#define IX_GAMEPAD_DPAD_DOWN      0x0002
#define IX_GAMEPAD_DPAD_LEFT      0x0004
#define IX_GAMEPAD_DPAD_RIGHT     0x0008
#define IX_GAMEPAD_START          0x0010
#define IX_GAMEPAD_BACK           0x0020
#define IX_GAMEPAD_LTHUMB         0x0040
#define IX_GAMEPAD_RTHUMB         0x0080
#define IX_GAMEPAD_LB             0x0100
#define IX_GAMEPAD_RB             0x0200
#define IX_GAMEPAD_A              0x1000
#define IX_GAMEPAD_B              0x2000
#define IX_GAMEPAD_X              0x4000
#define IX_GAMEPAD_Y              0x8000

// Eixos
#define IX_AXIS_LX    0
#define IX_AXIS_LY    1
#define IX_AXIS_RX    2
#define IX_AXIS_RY    3
#define IX_AXIS_LT    4
#define IX_AXIS_RT    5
#define IX_AXIS_COUNT 6

// Backend em uso por slot
#define IX_GAMEPAD_BACKEND_NONE    0
#define IX_GAMEPAD_BACKEND_XINPUT  1
#define IX_GAMEPAD_BACKEND_DINPUT  2

// ---------------------------------------------------------------------------
//  Lifecycle
// ---------------------------------------------------------------------------
void Gamepad_Initialize();
void Gamepad_Update();
void Gamepad_Shutdown();

// ---------------------------------------------------------------------------
//  Queries
// ---------------------------------------------------------------------------
int   Gamepad_IsConnected(int index);
int   Gamepad_IsButtonDown     (int index, unsigned int button);
int   Gamepad_WasButtonPressed (int index, unsigned int button);
int   Gamepad_WasButtonReleased(int index, unsigned int button);
float Gamepad_GetAxis          (int index, int axis);
int   Gamepad_IsTriggerDown    (int index, int axis);
int   Gamepad_WasTriggerPressed (int index, int axis);
int   Gamepad_WasTriggerReleased(int index, int axis);

// ---------------------------------------------------------------------------
//  Vibracao (no-op silencioso em controles sem suporte via DInput)
// ---------------------------------------------------------------------------
void  Gamepad_SetVibration(int index, float leftMotor, float rightMotor);
void  Gamepad_StopVibration(int index);

// ---------------------------------------------------------------------------
//  Configuracao
// ---------------------------------------------------------------------------
void  Gamepad_SetDeadZone(int index, float deadZone);
float Gamepad_GetDeadZone(int index);

// ---------------------------------------------------------------------------
//  Diagnostico
// ---------------------------------------------------------------------------
int         Gamepad_GetConnectedCount();
int         Gamepad_GetBackend(int index);  // IX_GAMEPAD_BACKEND_*
const char* Gamepad_GetName(int index);     // "Xbox Controller", "DualShock 4", etc.
const char* Gamepad_ButtonToString(unsigned int button);
const char* Gamepad_AxisToString(int axis);

#ifdef __cplusplus
}
#endif

#endif // IX_CORE_GAMEPAD_H
