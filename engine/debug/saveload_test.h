#ifndef IX_DEBUG_SAVELOAD_TEST_H
#define IX_DEBUG_SAVELOAD_TEST_H

void DebugSaveLoadTest_Initialize();
void DebugSaveLoadTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased);
void DebugSaveLoadTest_ApplyRendererState(float renderFPS, float frameMs);
int  DebugSaveLoadTest_ShouldReturnToMenu();
void DebugSaveLoadTest_ClearReturnToMenu();

#endif // IX_DEBUG_SAVELOAD_TEST_H
