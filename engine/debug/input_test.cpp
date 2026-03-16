#include "input_test.h"
#include "../renderer.h"
#include "../core/input.h"
#include <cstdio>

static int  g_returnToMenu = 0;
static char g_statusLine[256] = "Input ready.";
static char g_helpLine[256]   = "Move mouse | Click buttons | Press keys | Wheel scroll | ESC menu";

void DebugInputTest_Initialize()
{
    g_returnToMenu = 0;
    std::snprintf(g_statusLine, sizeof(g_statusLine), "Input ready.");
    std::snprintf(g_helpLine,   sizeof(g_helpLine),
        "Move mouse | Click buttons | Press keys | Wheel scroll | ESC menu");
}

void DebugInputTest_Update()
{
    int mouseX       = Input_GetMouseX();
    int mouseY       = Input_GetMouseY();
    int wheel        = Input_GetMouseWheelDelta();
    int lastPressed  = Input_GetLastPressedKey();
    int lastReleased = Input_GetLastReleasedKey();

    if (Input_WasKeyPressed(VK_ESCAPE))
        g_returnToMenu = 1;

    std::snprintf(g_statusLine, sizeof(g_statusLine),
        "Mouse %d,%d | L[%d/%d/%d] R[%d/%d/%d] M[%d/%d/%d] | Wheel %d | Last press %s (%d) | Last release %s (%d)",
        mouseX, mouseY,
        Input_IsMouseButtonDown(IX_MOUSE_LEFT),
        Input_WasMouseButtonPressed(IX_MOUSE_LEFT),
        Input_WasMouseButtonReleased(IX_MOUSE_LEFT),
        Input_IsMouseButtonDown(IX_MOUSE_RIGHT),
        Input_WasMouseButtonPressed(IX_MOUSE_RIGHT),
        Input_WasMouseButtonReleased(IX_MOUSE_RIGHT),
        Input_IsMouseButtonDown(IX_MOUSE_MIDDLE),
        Input_WasMouseButtonPressed(IX_MOUSE_MIDDLE),
        Input_WasMouseButtonReleased(IX_MOUSE_MIDDLE),
        wheel,
        Input_KeyToString(lastPressed),  lastPressed,
        Input_KeyToString(lastReleased), lastReleased);

    std::snprintf(g_helpLine, sizeof(g_helpLine),
        "Down: W[%d] A[%d] S[%d] D[%d] SPACE[%d] SHIFT[%d] CTRL[%d] | ESC menu",
        Input_IsKeyDown('W'),       Input_IsKeyDown('A'),
        Input_IsKeyDown('S'),       Input_IsKeyDown('D'),
        Input_IsKeyDown(VK_SPACE),  Input_IsKeyDown(VK_SHIFT),
        Input_IsKeyDown(VK_CONTROL));
}

void DebugInputTest_ApplyRendererState(float renderFPS, float frameMs)
{
    Renderer_SetOverlayText("InteractiveX r0.2", "Input debug", g_statusLine);
    Renderer_SetHelpText(g_helpLine);
    Renderer_SetDisplayOptions(1, 1, 1);
    Renderer_SetControlButton(NULL);
    Renderer_SetControlButtons(NULL, 0);
    Renderer_SetControlSliders(NULL, 0);
    Renderer_SetOverlayMetrics(renderFPS, frameMs, 0.0f);
}

int  DebugInputTest_ShouldReturnToMenu() { return g_returnToMenu; }
void DebugInputTest_ClearReturnToMenu()  { g_returnToMenu = 0;    }
