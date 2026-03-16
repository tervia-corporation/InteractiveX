#ifndef IX_PANEL_H
#define IX_PANEL_H

#ifdef __cplusplus
extern "C" {
#endif

struct IDirect3DTexture9;

// ---------------------------------------------------------------------------
//  IXPanel  —  container com header, fundo e borda totalmente customizaveis.
//
//  Fundo do corpo:
//      bgGradTop/Bottom com alpha > 0  ->  gradiente
//      bgTexture != NULL               ->  sprite (tintado por bgTextureTint)
//      caso contrario                  ->  cor solida (backgroundColor)
//
//  Header:
//      headerGradTop/Bottom com alpha > 0  ->  gradiente
//      caso contrario                      ->  cor solida (headerColor)
//
//  Forma:
//      cornerRadius > 0  ->  cantos chanfrados (octogono)
//      strokeWidth  > 0  ->  espessura da borda (default 1)
//
//  Uso rapido (igual ao antigo):
//      IXPanel p;
//      Panel_Init(&p, 10, 10, 300, 400, "Titulo");
//
//  Uso CSS:
//      Panel_SetBodyGradient(&p, 0xDD0A1020, 0xDD05080F);
//      Panel_SetHeaderGradient(&p, 0xFF1A3A5A, 0xFF0E2236);
//      Panel_SetBorderStyle(&p, 0xFF2A6A9A, 2);
//      Panel_SetCornerRadius(&p, 6);
//      Panel_SetTitleFont(&p, 20, 1, 1);  // 20px bold centrado
// ---------------------------------------------------------------------------

typedef struct IXPanel
{
    int x, y, width, height;

    // Titulo
    const char*  title;
    unsigned int titleColor;
    int          titleFontSize;    // 0 = padrao
    int          titleFontBold;    // 1 = negrito
    int          titleAlignment;   // 0=esq 1=centro 2=dir

    // Estrutura
    int padding;
    int headerHeight;

    // Fundo do corpo
    unsigned int backgroundColor;
    unsigned int bgGradTop;        // 0x00 = desativado
    unsigned int bgGradBottom;

    // Fundo sprite
    IDirect3DTexture9* bgTexture;  // NULL = sem sprite
    unsigned int       bgTextureTint;

    // Header
    unsigned int headerColor;
    unsigned int headerGradTop;    // 0x00 = desativado
    unsigned int headerGradBottom;

    // Borda
    unsigned int borderColor;
    int          strokeWidth;      // default 1
    int          cornerRadius;     // 0 = quadrado

} IXPanel;

// --- Inicializacao ---
void Panel_Init(IXPanel* panel, int x, int y, int width, int height, const char* title);
void Panel_SetTitle(IXPanel* panel, const char* title);
void Panel_SetBounds(IXPanel* panel, int x, int y, int width, int height);
int  Panel_IsInside(const IXPanel* panel, int pointX, int pointY);

// --- Fundo do corpo ---
void Panel_SetBackground(IXPanel* panel, unsigned int color);
void Panel_SetBodyGradient(IXPanel* panel, unsigned int top, unsigned int bottom);
void Panel_SetBodySprite(IXPanel* panel, IDirect3DTexture9* texture, unsigned int tint);

// --- Header ---
void Panel_SetHeaderColor(IXPanel* panel, unsigned int color);
void Panel_SetHeaderGradient(IXPanel* panel, unsigned int top, unsigned int bottom);
void Panel_SetHeaderHeight(IXPanel* panel, int height);

// --- Titulo ---
void Panel_SetTitleFont(IXPanel* panel, int fontSize, int bold, int alignment);
void Panel_SetTitleColor(IXPanel* panel, unsigned int color);

// --- Borda e forma ---
void Panel_SetBorderStyle(IXPanel* panel, unsigned int color, int strokeWidth);
void Panel_SetCornerRadius(IXPanel* panel, int radius);

#ifdef __cplusplus
}
#endif

#endif // IX_PANEL_H
