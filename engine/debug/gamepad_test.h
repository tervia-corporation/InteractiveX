#ifndef IX_DEBUG_GAMEPAD_TEST_H
#define IX_DEBUG_GAMEPAD_TEST_H

void DebugGamepadTest_Initialize();
void DebugGamepadTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased);
void DebugGamepadTest_Render();
void DebugGamepadTest_ApplyRendererState(float renderFPS, float frameMs);
int  DebugGamepadTest_ShouldReturnToMenu();
void DebugGamepadTest_ClearReturnToMenu();

#endif
