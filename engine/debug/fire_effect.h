#ifndef IX_DEBUG_FIRE_EFFECT_H
#define IX_DEBUG_FIRE_EFFECT_H

#include <windows.h>

void DebugFireEffect_Initialize();
void DebugFireEffect_InvalidateLayout();
void DebugFireEffect_HandleMouseMove(int mouseX, int mouseY);
void DebugFireEffect_BeginMouseDrag(int mouseX, int mouseY);
void DebugFireEffect_EndMouseDrag();
void DebugFireEffect_HandleKeyDown(WPARAM key);
void DebugFireEffect_UpdateUI(int mouseX, int mouseY, int mouseDown, int mouseReleased);
void DebugFireEffect_ApplyRendererState(float renderFPS, float frameMs, float simUPS);
int  DebugFireEffect_IsPointOverUI(int x, int y);
int  DebugFireEffect_IsDraggingFire();
void DebugFireEffect_AccumulateSimulation(double deltaMs, unsigned int* simStepsOut);

// Viewport is now owned by this module, not the renderer.
// scene.cpp calls this to get dst rect for Renderer_DrawPixelBuffer.
void DebugFireEffect_GetViewport(int* x, int* y, int* w, int* h);

const unsigned int* DebugFireEffect_GetPixelBuffer();
int  DebugFireEffect_GetBufferWidth();
int  DebugFireEffect_GetBufferHeight();
int  DebugFireEffect_ShouldReturnToMenu();
void DebugFireEffect_ClearReturnToMenu();

#endif
