#ifndef IX_DEBUG_CAMERA_TEST_H
#define IX_DEBUG_CAMERA_TEST_H

void DebugCameraTest_Initialize();
void DebugCameraTest_InvalidateLayout();
void DebugCameraTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased);
void DebugCameraTest_Render();
void DebugCameraTest_ApplyRendererState(float renderFPS, float frameMs);
void DebugCameraTest_HandleKeyDown(unsigned int key);
int  DebugCameraTest_ShouldReturnToMenu();
void DebugCameraTest_ClearReturnToMenu();

#endif // IX_DEBUG_CAMERA_TEST_H
