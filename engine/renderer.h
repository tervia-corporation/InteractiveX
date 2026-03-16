#ifndef IX_RENDERER_H
#define IX_RENDERER_H

#include <windows.h>
#include "interface/button.h"
#include "interface/slider.h"

struct IDirect3DDevice9;
struct IDirect3DTexture9;

// ---------------------------------------------------------------------------
//  Core lifecycle
// ---------------------------------------------------------------------------
bool Renderer_Initialize(HWND hWnd);
void Renderer_HandleResize(int width, int height);
void Renderer_Cleanup();

// ---------------------------------------------------------------------------
//  Queries
// ---------------------------------------------------------------------------
void              Renderer_GetClientSize(int* width, int* height);

// Area total do conteudo do painel overlay (abaixo do header).
void              Renderer_GetOverlayContentRect(int* x, int* y, int* width, int* height);

// Area abaixo das linhas de texto do overlay — use esta para posicionar botoes.
// Calcula automaticamente baseado em quais linhas estao sendo mostradas
// (subtitle, status, metrics, help). Invalida automaticamente quando o
// overlay muda de estado.
void              Renderer_GetOverlayButtonsRect(int* x, int* y, int* width, int* height);

IDirect3DDevice9* Renderer_GetDevice();

// ---------------------------------------------------------------------------
//  Static display image
// ---------------------------------------------------------------------------
void Renderer_SetDisplayImage(IDirect3DTexture9* texture, int width, int height);
void Renderer_ClearDisplayImage();

// ---------------------------------------------------------------------------
//  Background image
// ---------------------------------------------------------------------------
bool Renderer_LoadBackgroundImage(const char* path);
void Renderer_ClearBackgroundImage();

// ---------------------------------------------------------------------------
//  Frame lifecycle
// ---------------------------------------------------------------------------
void Renderer_BeginFrame();
void Renderer_DrawPixelBuffer(const unsigned int* pixels,
                               int srcWidth,  int srcHeight,
                               int dstX,      int dstY,
                               int dstWidth,  int dstHeight);
void Renderer_DrawSprite(IDirect3DTexture9* texture,
                          int srcX, int srcY, int srcW, int srcH,
                          int dstX, int dstY, int dstW, int dstH,
                          unsigned int tint);
void Renderer_EndFrame();

// ---------------------------------------------------------------------------
//  Overlay panel configuration
// ---------------------------------------------------------------------------
void Renderer_SetOverlayText(const char* title,
                              const char* subtitle,
                              const char* statusLine);
void Renderer_SetOverlayMetrics(float renderFps, float frameMs, float simUps);

// Aceita qualquer quantidade de botoes — o renderer redimensiona automaticamente.
void Renderer_SetControlButton(const IXButton* button);
void Renderer_SetControlButtons(const IXButton** buttons, int count);
void Renderer_SetControlSliders(const IXSlider** sliders, int count);

void Renderer_SetDisplayOptions(int showFPS, int showHelp, int drawBackgroundGrid);
void Renderer_SetBackgroundColor(unsigned int argb);
void Renderer_SetHelpText(const char* helpText);

// Mostra ou oculta o painel overlay completo (util durante splash screen, cutscenes, etc.)
// 1 = visivel (padrao), 0 = oculto
void Renderer_SetOverlayEnabled(int enabled);

// ---------------------------------------------------------------------------
//  Video (chamado pelo modulo Video)
// ---------------------------------------------------------------------------

// Ativa ou desativa VSync. Tem efeito no proximo Renderer_HandleResize.
// 1 = VSync (D3DPRESENT_INTERVAL_ONE), 0 = imediato (IMMEDIATE).
void Renderer_SetVSync(int vsync);

#endif // IX_RENDERER_H
