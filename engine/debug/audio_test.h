#ifndef IX_DEBUG_AUDIO_TEST_H
#define IX_DEBUG_AUDIO_TEST_H

void DebugAudioTest_Initialize();
void DebugAudioTest_InvalidateLayout();
void DebugAudioTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased);
void DebugAudioTest_ApplyRendererState(float renderFPS, float frameMs);
int  DebugAudioTest_ShouldReturnToMenu();
void DebugAudioTest_ClearReturnToMenu();

#endif
