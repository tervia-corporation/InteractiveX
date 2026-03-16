#ifndef IX_DEBUG_ANIM_TEST_H
#define IX_DEBUG_ANIM_TEST_H

void DebugAnimTest_Initialize();
void DebugAnimTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased);
void DebugAnimTest_Render();
void DebugAnimTest_ApplyRendererState(float renderFPS, float frameMs);
void DebugAnimTest_HandleKeyDown(unsigned int key);
int  DebugAnimTest_ShouldReturnToMenu();
void DebugAnimTest_ClearReturnToMenu();

#endif // IX_DEBUG_ANIM_TEST_H
