#ifndef IX_DEBUG_PARTICLES_TEST_H
#define IX_DEBUG_PARTICLES_TEST_H

void DebugParticlesTest_Initialize();
void DebugParticlesTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased);
void DebugParticlesTest_Render();
void DebugParticlesTest_ApplyRendererState(float renderFPS, float frameMs);
void DebugParticlesTest_HandleKeyDown(unsigned int key);
int  DebugParticlesTest_ShouldReturnToMenu();
void DebugParticlesTest_ClearReturnToMenu();

#endif
