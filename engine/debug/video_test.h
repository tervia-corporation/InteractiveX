#ifndef IX_DEBUG_VIDEO_TEST_H
#define IX_DEBUG_VIDEO_TEST_H

void DebugVideoTest_Initialize();
void DebugVideoTest_InvalidateLayout();
void DebugVideoTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased);
void DebugVideoTest_Render();
void DebugVideoTest_ApplyRendererState(float renderFPS, float frameMs);
int  DebugVideoTest_ShouldReturnToMenu();
void DebugVideoTest_ClearReturnToMenu();

#endif // IX_DEBUG_VIDEO_TEST_H
