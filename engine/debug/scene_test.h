#ifndef IX_DEBUG_SCENE_TEST_H
#define IX_DEBUG_SCENE_TEST_H

void DebugSceneTest_Initialize();
void DebugSceneTest_InvalidateLayout();
void DebugSceneTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased);
void DebugSceneTest_ApplyRendererState(float renderFPS, float frameMs);
int  DebugSceneTest_ShouldReturnToMenu();
void DebugSceneTest_ClearReturnToMenu();

#endif
