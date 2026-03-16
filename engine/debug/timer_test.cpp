#include "timer_test.h"
#include "../renderer.h"
#include "../core/input.h"
#include "../core/timer.h"
#include <cstdio>

static int  g_returnToMenu = 0;
static char g_statusLine[256] = "Timer ready.";
static char g_helpLine[256]   = "Timer module stats | ESC menu";

void DebugTimerTest_Initialize()
{
    g_returnToMenu = 0;
    std::snprintf(g_statusLine, sizeof(g_statusLine), "Timer ready.");
    std::snprintf(g_helpLine,   sizeof(g_helpLine),   "Timer module stats | ESC menu");
}

void DebugTimerTest_Update()
{
    if (Input_WasKeyPressed(VK_ESCAPE))
        g_returnToMenu = 1;

    std::snprintf(g_statusLine, sizeof(g_statusLine),
        "Delta %.3f ms | Smoothed %.3f ms | Render FPS %.2f | Sim UPS %.2f | Frame steps %u",
        Timer_GetDeltaMilliseconds(),
        Timer_GetSmoothedFrameMilliseconds(),
        Timer_GetRenderFPS(),
        Timer_GetSimUPS(),
        Timer_GetFrameSimulationSteps());

    std::snprintf(g_helpLine, sizeof(g_helpLine),
        "Runtime %.2f s | Frames %u | Delta %.6f s | ESC menu",
        Timer_GetTotalSeconds(),
        Timer_GetFrameCount(),
        Timer_GetDeltaSeconds());
}

void DebugTimerTest_ApplyRendererState()
{
    Renderer_SetOverlayText("InteractiveX r0.2", "Timer debug", g_statusLine);
    Renderer_SetHelpText(g_helpLine);
    Renderer_SetDisplayOptions(1, 1, 1);
    Renderer_SetControlButton(NULL);
    Renderer_SetControlButtons(NULL, 0);
    Renderer_SetControlSliders(NULL, 0);
    Renderer_SetOverlayMetrics(
        Timer_GetRenderFPS(),
        Timer_GetSmoothedFrameMilliseconds(),
        Timer_GetSimUPS());
}

int  DebugTimerTest_ShouldReturnToMenu() { return g_returnToMenu; }
void DebugTimerTest_ClearReturnToMenu()  { g_returnToMenu = 0;    }
