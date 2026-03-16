#ifndef IX_DEBUG_ENTITY_TEST_H
#define IX_DEBUG_ENTITY_TEST_H

void DebugEntityTest_Initialize();
void DebugEntityTest_InvalidateLayout();
void DebugEntityTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased);
void DebugEntityTest_Render();
void DebugEntityTest_ApplyRendererState(float renderFPS, float frameMs);
void DebugEntityTest_HandleKeyDown(unsigned int key);
int  DebugEntityTest_ShouldReturnToMenu();
void DebugEntityTest_ClearReturnToMenu();

#endif // IX_DEBUG_ENTITY_TEST_H
