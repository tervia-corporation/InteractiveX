#ifndef IX_CORE_ENGINE_H
#define IX_CORE_ENGINE_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

bool Engine_Initialize(HWND hWnd);
void Engine_Shutdown();
void Engine_HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void Engine_RunFrame();

#ifdef __cplusplus
}
#endif

#endif
