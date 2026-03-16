#ifndef IX_DEBUG_IMAGE_TEST_H
#define IX_DEBUG_IMAGE_TEST_H

void DebugImageTest_Initialize();
void DebugImageTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased);
void DebugImageTest_Render();
void DebugImageTest_ApplyRendererState(float renderFPS, float frameMs);
int  DebugImageTest_ShouldReturnToMenu();
void DebugImageTest_ClearReturnToMenu();

#endif // IX_DEBUG_IMAGE_TEST_H
