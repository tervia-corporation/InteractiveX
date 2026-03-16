#ifndef IX_GUI_H
#define IX_GUI_H

#include <windows.h>
#include "interface/button.h"
#include "interface/slider.h"
#include "interface/panel.h"
#include "interface/label.h"

struct IDirect3DDevice9;
struct IDirect3DTexture9;

// ---------------------------------------------------------------------------
//  Lifecycle (interno)
// ---------------------------------------------------------------------------
bool GUI_Initialize();
bool GUI_InternalCreate(IDirect3DDevice9* device);
void GUI_BeginFrame();
void GUI_EndFrame();
void GUI_Cleanup();

// ---------------------------------------------------------------------------
//  Texto — screen space
// ---------------------------------------------------------------------------
// Linha simples na fonte padrao (18px bold). Compatibilidade total.
void GUI_DrawTextLine(int x, int y, const char* text, unsigned int color = 0xFFFFFFFF);

// Texto com controle completo.
// fontSize=0 usa padrao. alignment: 0=esq 1=centro 2=dir. wrap=1 quebra linha.
void GUI_DrawTextEx(int x, int y, int width, int height,
                    const char* text, unsigned int color,
                    int alignment, int fontSize, int bold, int wrap);

// ---------------------------------------------------------------------------
//  Texto — world space
//
//  Converte coordenadas de mundo para tela usando a camera ativa
//  (Camera_GetActive). Se nenhuma camera estiver attached, worldX/Y sao
//  usados diretamente como coordenadas de tela.
//
//  anchorX/anchorY: pivot do texto em [0,1].
//      0.5, 1.0 = centralizado horizontalmente, ancorado na base
//      0.5, 0.5 = centralizado no ponto
//      0.0, 0.0 = canto top-left no ponto (comportamento de DrawTextLine)
//
//  Uso tipico (HP flutuante acima de entidade):
//      GUI_DrawWorldText(e->x + e->w/2, e->y - 4,
//                        "100 HP", 0xFFFF4444,
//                        0.5f, 1.0f,       // anchorado em baixo, centrado
//                        0, 14, 0);        // alinhamento auto, 14px normal
// ---------------------------------------------------------------------------
void GUI_DrawWorldText(float worldX, float worldY,
                       const char* text, unsigned int color,
                       float anchorX, float anchorY,
                       int alignment, int fontSize, int bold);

// ---------------------------------------------------------------------------
//  Geometria
// ---------------------------------------------------------------------------
void GUI_DrawFilledRect(int x, int y, int width, int height, unsigned int color);
void GUI_DrawOutlinedRect(int x, int y, int width, int height, unsigned int color);
void GUI_DrawGradientRect(int x, int y, int width, int height,
                           unsigned int colorTop, unsigned int colorBottom);
void GUI_DrawChamferRect(int x, int y, int width, int height, int radius,
                          unsigned int color);
void GUI_DrawChamferGradientRect(int x, int y, int width, int height, int radius,
                                  unsigned int colorTop, unsigned int colorBottom);
void GUI_DrawChamferOutline(int x, int y, int width, int height, int radius,
                             int strokeWidth, unsigned int color);
void GUI_DrawSprite(IDirect3DTexture9* texture,
                    int srcX, int srcY, int srcW, int srcH,
                    int dstX, int dstY, int dstW, int dstH,
                    unsigned int tint);

// ---------------------------------------------------------------------------
//  Widgets
// ---------------------------------------------------------------------------
void GUI_DrawButton(const IXButton* button);
void GUI_DrawSlider(const IXSlider* slider);
void GUI_DrawPanel(const IXPanel* panel);
void GUI_DrawLabel(const IXLabel* label);

#endif // IX_GUI_H
