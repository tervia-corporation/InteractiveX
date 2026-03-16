#include "gamepad_test.h"
#include "../core/gamepad.h"
#include "../core/input.h"
#include "../core/timer.h"
#include "../renderer.h"
#include "../gui.h"
#include <cstdio>
#include <cstring>

// ---------------------------------------------------------------------------
//  Layout
// ---------------------------------------------------------------------------
#define PAD_AREA_X   384
#define PAD_AREA_Y    16
#define PAD_W        278
#define PAD_H        258
#define PAD_GAP        6

// ---------------------------------------------------------------------------
//  Estado
// ---------------------------------------------------------------------------
static int  g_returnToMenu = 0;
static char g_statusLine[256] = "Gamepad test — conecte Xbox, DS4, DualSense ou Switch Pro";
static char g_helpLine[128]   = "F vibrar pad 0 | ESC menu";

static float g_vibrateTimer = 0.0f;

// ---------------------------------------------------------------------------
//  Helpers de desenho
// ---------------------------------------------------------------------------
static void DrawBar(int x, int y, int w, int h,
                    float value,         // -1..1 para eixos, 0..1 para gatilhos
                    unsigned int color,
                    int bipolar)         // 1 = barra central, 0 = barra cheia
{
    // Fundo
    GUI_DrawFilledRect(x, y, w, h, 0x33FFFFFF);
    GUI_DrawOutlinedRect(x, y, w, h, 0x44FFFFFF);

    if (bipolar)
    {
        int center = x + w / 2;
        if (value >= 0.0f)
        {
            int fillW = (int)(value * (w / 2));
            if (fillW > 0) GUI_DrawFilledRect(center, y + 1, fillW, h - 2, color);
        }
        else
        {
            int fillW = (int)(-value * (w / 2));
            if (fillW > 0) GUI_DrawFilledRect(center - fillW, y + 1, fillW, h - 2, color);
        }
        // Linha central
        GUI_DrawFilledRect(center, y, 1, h, 0x88FFFFFF);
    }
    else
    {
        int fillW = (int)(value * (w - 2));
        if (fillW > 0) GUI_DrawFilledRect(x + 1, y + 1, fillW, h - 2, color);
    }
}

static void DrawButton(int x, int y, int size,
                        int isDown, int wasPressed,
                        const char* label,
                        unsigned int colorActive)
{
    unsigned int bg = isDown    ? colorActive :
                      wasPressed ? 0xFFFFFFFF  : 0x22FFFFFF;
    GUI_DrawFilledRect(x, y, size, size, bg);
    GUI_DrawOutlinedRect(x, y, size, size, 0x66FFFFFF);
    if (label && label[0])
        GUI_DrawTextLine(x + 2, y + 1, label, isDown ? 0xFF000000 : 0xFFCCCCCC);
}

static void DrawPad(int padX, int padY, int padIndex)
{
    int connected = Gamepad_IsConnected(padIndex);

    // Fundo do card
    unsigned int bg = connected ? 0x22FFFFFF : 0x11FFFFFF;
    GUI_DrawFilledRect(padX, padY, PAD_W, PAD_H, bg);
    GUI_DrawOutlinedRect(padX, padY, PAD_W, PAD_H, connected ? 0x55FFFFFF : 0x22FFFFFF);

    // Titulo
    char title[32];
    int backend = Gamepad_GetBackend(padIndex);
    const char* bname = (backend == IX_GAMEPAD_BACKEND_XINPUT) ? "XInput" : (backend == IX_GAMEPAD_BACKEND_DINPUT) ? "DInput" : "";
    std::snprintf(title, sizeof(title), "Pad %d %s", padIndex + 1, bname);
    GUI_DrawTextLine(padX + 8, padY + 6,
        title,
        connected ? 0xFF88FF88 : 0xFF444444);

    // Nome do dispositivo
    GUI_DrawTextLine(padX + 8, padY + 22,
        Gamepad_GetName(padIndex),
        connected ? 0xFF888888 : 0xFF333333);

    if (!connected)
    {
        GUI_DrawTextLine(padX + 8, padY + 38, "desconectado", 0xFF444444);
        return;
    }

    int cx = padX + 8;
    int cy = padY + 40;

    // --- Eixos analogicos ---
    const struct { int axis; const char* name; int bipolar; unsigned int color; } AXES[] = {
        { IX_AXIS_LX, "LX", 1, 0xFF4488FF },
        { IX_AXIS_LY, "LY", 1, 0xFF4488FF },
        { IX_AXIS_RX, "RX", 1, 0xFF88FF44 },
        { IX_AXIS_RY, "RY", 1, 0xFF88FF44 },
        { IX_AXIS_LT, "LT", 0, 0xFFFFAA44 },
        { IX_AXIS_RT, "RT", 0, 0xFFFFAA44 },
    };

    // Coluna: label(24px) | barra(110px) | valor(48px)
    for (int a = 0; a < 6; a++)
    {
        float v = Gamepad_GetAxis(padIndex, AXES[a].axis);
        char val[12];
        std::snprintf(val, sizeof(val), "%.2f", v);
        GUI_DrawTextLine(cx,       cy, AXES[a].name, 0xFFAAAAAA);
        DrawBar(cx + 24, cy + 2, 110, 12, v, AXES[a].color, AXES[a].bipolar);
        GUI_DrawTextLine(cx + 138, cy, val,           0xFFCCCCCC);
        cy += 17;
    }

    cy += 6;

    // --- Linha 1: face buttons + LB/RB ---
    const struct { unsigned int btn; const char* lbl; unsigned int color; } FACE[] = {
        { IX_GAMEPAD_A, "A", 0xFF44BB44 },
        { IX_GAMEPAD_B, "B", 0xFFBB4444 },
        { IX_GAMEPAD_X, "X", 0xFF4444BB },
        { IX_GAMEPAD_Y, "Y", 0xFFBBAA22 },
    };
    for (int b = 0; b < 4; b++)
        DrawButton(cx + b * 24, cy, 22,
            Gamepad_IsButtonDown(padIndex, FACE[b].btn),
            Gamepad_WasButtonPressed(padIndex, FACE[b].btn),
            FACE[b].lbl, FACE[b].color);

    DrawButton(cx + 106, cy, 24, Gamepad_IsButtonDown(padIndex, IX_GAMEPAD_LB), 0, "LB", 0xFF888888);
    DrawButton(cx + 134, cy, 24, Gamepad_IsButtonDown(padIndex, IX_GAMEPAD_RB), 0, "RB", 0xFF888888);

    cy += 26;

    // --- Linha 2: DPad + Back/Start/L3/R3 + LT/RT ---
    DrawButton(cx + 18, cy,      18, Gamepad_IsButtonDown(padIndex, IX_GAMEPAD_DPAD_UP),    0, "^", 0xFF888888);
    DrawButton(cx,      cy + 20, 18, Gamepad_IsButtonDown(padIndex, IX_GAMEPAD_DPAD_LEFT),  0, "<", 0xFF888888);
    DrawButton(cx + 18, cy + 20, 18, Gamepad_IsButtonDown(padIndex, IX_GAMEPAD_DPAD_DOWN),  0, "v", 0xFF888888);
    DrawButton(cx + 36, cy + 20, 18, Gamepad_IsButtonDown(padIndex, IX_GAMEPAD_DPAD_RIGHT), 0, ">", 0xFF888888);

    DrawButton(cx + 66, cy,      24, Gamepad_IsButtonDown(padIndex, IX_GAMEPAD_BACK),   0, "Bk", 0xFF666666);
    DrawButton(cx + 94, cy,      24, Gamepad_IsButtonDown(padIndex, IX_GAMEPAD_START),  0, "St", 0xFF666666);
    DrawButton(cx + 66, cy + 27, 24, Gamepad_IsButtonDown(padIndex, IX_GAMEPAD_LTHUMB), 0, "L3", 0xFF666688);
    DrawButton(cx + 94, cy + 27, 24, Gamepad_IsButtonDown(padIndex, IX_GAMEPAD_RTHUMB), 0, "R3", 0xFF666688);

    DrawButton(cx + 134, cy,      24, Gamepad_IsTriggerDown(padIndex, IX_AXIS_LT), 0, "LT", 0xFFFFAA44);
    DrawButton(cx + 162, cy,      24, Gamepad_IsTriggerDown(padIndex, IX_AXIS_RT), 0, "RT", 0xFFFFAA44);
}

// ---------------------------------------------------------------------------
//  Public API
// ---------------------------------------------------------------------------
void DebugGamepadTest_Initialize()
{
    g_returnToMenu = 0;
    g_vibrateTimer = 0.0f;
}

void DebugGamepadTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased)
{
    (void)mouseX; (void)mouseY; (void)mouseDown; (void)mouseReleased;

    if (Input_WasKeyPressed(VK_ESCAPE)) { g_returnToMenu = 1; return; }

    float dt = Timer_GetDeltaSeconds();

    // F = vibra pad 0 por 0.5s
    if (Input_WasKeyPressed('F') && Gamepad_IsConnected(0))
    {
        Gamepad_SetVibration(0, 0.6f, 1.0f);
        g_vibrateTimer = 0.5f;
    }

    // Para vibracao apos timer
    if (g_vibrateTimer > 0.0f)
    {
        g_vibrateTimer -= dt;
        if (g_vibrateTimer <= 0.0f)
        {
            g_vibrateTimer = 0.0f;
            Gamepad_StopVibration(0);
        }
    }

    // Mostra ultimo botao pressionado em qualquer pad
    static char lastEvent[64] = "";
    for (int p = 0; p < IX_GAMEPAD_MAX; p++)
    {
        if (!Gamepad_IsConnected(p)) continue;
        unsigned int allBtns[] = {
            IX_GAMEPAD_A, IX_GAMEPAD_B, IX_GAMEPAD_X, IX_GAMEPAD_Y,
            IX_GAMEPAD_LB, IX_GAMEPAD_RB, IX_GAMEPAD_START, IX_GAMEPAD_BACK,
            IX_GAMEPAD_DPAD_UP, IX_GAMEPAD_DPAD_DOWN,
            IX_GAMEPAD_DPAD_LEFT, IX_GAMEPAD_DPAD_RIGHT,
            IX_GAMEPAD_LTHUMB, IX_GAMEPAD_RTHUMB
        };
        for (int b = 0; b < 14; b++)
        {
            if (Gamepad_WasButtonPressed(p, allBtns[b]))
                std::snprintf(lastEvent, sizeof(lastEvent),
                    "Pad %d: %s pressionado", p + 1,
                    Gamepad_ButtonToString(allBtns[b]));
        }
        if (Gamepad_WasTriggerPressed(p, IX_AXIS_LT))
            std::snprintf(lastEvent, sizeof(lastEvent), "Pad %d: LT pressionado", p + 1);
        if (Gamepad_WasTriggerPressed(p, IX_AXIS_RT))
            std::snprintf(lastEvent, sizeof(lastEvent), "Pad %d: RT pressionado", p + 1);
    }

    int total = Gamepad_GetConnectedCount();
    std::snprintf(g_statusLine, sizeof(g_statusLine),
        "%d controle(s) conectado(s)%s%s",
        total,
        lastEvent[0] ? " | " : "",
        lastEvent);
}

void DebugGamepadTest_Render()
{
    int cw = 0, ch = 0;
    Renderer_GetClientSize(&cw, &ch);

    // Desenha os 4 cards em 2x2
    for (int i = 0; i < IX_GAMEPAD_MAX; i++)
    {
        int col = i % 2;
        int row = i / 2;
        int px  = PAD_AREA_X + col * (PAD_W + PAD_GAP);
        int py  = PAD_AREA_Y + row * (PAD_H + PAD_GAP);
        DrawPad(px, py, i);
    }
}

void DebugGamepadTest_ApplyRendererState(float renderFPS, float frameMs)
{
    Renderer_SetOverlayText("InteractiveX r0.2", "Gamepad test", g_statusLine);
    Renderer_SetHelpText(g_helpLine);
    Renderer_SetDisplayOptions(1, 1, 0);
    Renderer_SetControlButton(NULL);
    Renderer_SetControlButtons(NULL, 0);
    Renderer_SetControlSliders(NULL, 0);
    Renderer_SetOverlayMetrics(renderFPS, frameMs, 0.0f);
}

int  DebugGamepadTest_ShouldReturnToMenu() { return g_returnToMenu; }
void DebugGamepadTest_ClearReturnToMenu()
{
    // Para vibracao ao sair
    for (int i = 0; i < IX_GAMEPAD_MAX; i++)
        Gamepad_StopVibration(i);
    g_returnToMenu = 0;
}
