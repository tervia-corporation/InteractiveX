#ifndef IX_DEBUG_MATH2D_TEST_H
#define IX_DEBUG_MATH2D_TEST_H

void DebugMath2DTest_Initialize();
void DebugMath2DTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased);
void DebugMath2DTest_Render();
void DebugMath2DTest_ApplyRendererState(float renderFPS, float frameMs);
int  DebugMath2DTest_ShouldReturnToMenu();
void DebugMath2DTest_ClearReturnToMenu();

#endif
