#ifndef IX_DEBUG_TILEMAP_TEST_H
#define IX_DEBUG_TILEMAP_TEST_H

void DebugTilemapTest_Initialize();
void DebugTilemapTest_InvalidateLayout();
void DebugTilemapTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased);
void DebugTilemapTest_Render();
void DebugTilemapTest_ApplyRendererState(float renderFPS, float frameMs);
void DebugTilemapTest_HandleKeyDown(unsigned int key);
int  DebugTilemapTest_ShouldReturnToMenu();
void DebugTilemapTest_ClearReturnToMenu();

#endif // IX_DEBUG_TILEMAP_TEST_H
