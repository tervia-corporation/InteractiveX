#include "engine.h"
#include "scene.h"
#include "input.h"
#include "gamepad.h"
#include "timer.h"
#include "assets.h"
#include "transition.h"
#include "../renderer.h"
#include "../media/audio.h"
#include "../debug/debug_scenes.h"

namespace
{
    HWND g_engineWindow = NULL;
}

bool Engine_Initialize(HWND hWnd)
{
    g_engineWindow = hWnd;
    if (!Renderer_Initialize(hWnd))   return false;
    Input_Initialize();
    Gamepad_Initialize();
    Timer_Initialize();
    if (!Assets_Initialize())         return false;
    if (!MediaAudio_Initialize(hWnd)) return false;
    Scene_Initialize();
    Scene_RegisterDebugScenes();
    Scene_SetCurrent(IX_SCENE_MENU);
    Scene_OnEnterCurrent();
    return true;
}

void Engine_Shutdown()
{
    Gamepad_Shutdown();
    MediaAudio_Shutdown();
    Assets_Shutdown();
    Renderer_Cleanup();
    g_engineWindow = NULL;
}

void Engine_HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    (void)hWnd;
    Input_HandleMessage(msg, wParam, lParam);
    switch (msg)
    {
        case WM_MOUSEMOVE:
            Scene_HandleMouseMove(Input_GetMouseX(), Input_GetMouseY());
            break;
        case WM_LBUTTONDOWN:
            SetCapture(hWnd);
            Scene_BeginMouseDrag(Input_GetMouseX(), Input_GetMouseY());
            break;
        case WM_LBUTTONUP:
            ReleaseCapture();
            Scene_EndMouseDrag();
            break;
        case WM_KEYDOWN:
            Scene_HandleKeyDown(wParam);
            break;
        case WM_CAPTURECHANGED:
            Scene_EndMouseDrag();
            break;
        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED)
            {
                Renderer_HandleResize((int)LOWORD(lParam), (int)HIWORD(lParam));
                Scene_InvalidateLayout();
            }
            break;
    }
}

void Engine_RunFrame()
{
    Timer_BeginFrame();
    Gamepad_Update();
    Transition_Update(Timer_GetDeltaSeconds());
    Scene_Update();
    if (!Transition_IsActive() && Scene_HasPendingChange())
    {
        Scene_SetCurrent(Scene_ConsumePendingChange());
        Scene_OnEnterCurrent();
    }
    Scene_Render();
    Timer_EndFrame();
    Input_BeginFrame();
    Sleep(1);
}
