#ifndef IX_DEBUG_RANDOM_TEST_H
#define IX_DEBUG_RANDOM_TEST_H

void DebugRandomTest_Initialize();
void DebugRandomTest_InvalidateLayout();
void DebugRandomTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased);
void DebugRandomTest_Render();
void DebugRandomTest_ApplyRendererState(float renderFPS, float frameMs);
int  DebugRandomTest_ShouldReturnToMenu();
void DebugRandomTest_ClearReturnToMenu();

#endif // IX_DEBUG_RANDOM_TEST_H
