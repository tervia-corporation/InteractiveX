#ifndef IX_DEBUG_HUHLU_MODULES_TEST_H
#define IX_DEBUG_HUHLU_MODULES_TEST_H

void DebugHuhluModules_Initialize();
void DebugHuhluModules_InvalidateLayout();
void DebugHuhluModules_Update(int mx,int my,int md,int mr);
void DebugHuhluModules_ApplyRendererState(float fps,float frameMs);
int  DebugHuhluModules_ShouldReturnToMenu();
void DebugHuhluModules_ClearReturnToMenu();

#endif
