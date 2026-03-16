#ifndef IX_DEBUG_INPUT_TEST_H
#define IX_DEBUG_INPUT_TEST_H

void DebugInputTest_Initialize();
void DebugInputTest_Update();
void DebugInputTest_ApplyRendererState(float renderFPS, float frameMs);
int  DebugInputTest_ShouldReturnToMenu();
void DebugInputTest_ClearReturnToMenu();

#endif
