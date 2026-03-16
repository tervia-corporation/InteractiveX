#ifndef IX_CORE_INPUT_H
#define IX_CORE_INPUT_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

enum IXMouseButton
{
    IX_MOUSE_LEFT = 0,
    IX_MOUSE_RIGHT = 1,
    IX_MOUSE_MIDDLE = 2,
    IX_MOUSE_BUTTON_COUNT = 3
};

void Input_Initialize();
void Input_BeginFrame();
void Input_HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

int Input_GetMouseX();
int Input_GetMouseY();
int Input_GetMouseWheelDelta();

int Input_IsMouseButtonDown(int button);
int Input_WasMouseButtonPressed(int button);
int Input_WasMouseButtonReleased(int button);

int Input_IsKeyDown(int vk);
int Input_WasKeyPressed(int vk);
int Input_WasKeyReleased(int vk);

int Input_GetLastPressedKey();
int Input_GetLastReleasedKey();
const char* Input_KeyToString(int vk);

#ifdef __cplusplus
}
#endif

#endif
