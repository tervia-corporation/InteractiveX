#ifndef IX_DEBUG_HUHLU_TEST_H
#define IX_DEBUG_HUHLU_TEST_H

void DebugHuhlu_Initialize();
void DebugHuhlu_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased);
void DebugHuhlu_Render();
void DebugHuhlu_ApplyRendererState(float fps, float frameMs);
int  DebugHuhlu_ShouldReturnToMenu();
void DebugHuhlu_ClearReturnToMenu();
void DebugHuhlu_HandleKeyDown(unsigned int key);

#endif
