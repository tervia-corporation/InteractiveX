#ifndef IX_DEBUG_TRANSITION_TEST_H
#define IX_DEBUG_TRANSITION_TEST_H

void DebugTransitionTest_Initialize();
void DebugTransitionTest_InvalidateLayout();
void DebugTransitionTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased);
void DebugTransitionTest_ApplyRendererState(float renderFPS, float frameMs);
int  DebugTransitionTest_ShouldReturnToMenu();
void DebugTransitionTest_ClearReturnToMenu();

#endif // IX_DEBUG_TRANSITION_TEST_H
