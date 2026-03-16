#ifndef IX_LABEL_H
#define IX_LABEL_H

#ifdef __cplusplus
extern "C" {
#endif

struct IDirect3DTexture9;

typedef enum IXLabelAlign
{
    IX_LABEL_ALIGN_LEFT   = 0,
    IX_LABEL_ALIGN_CENTER = 1,
    IX_LABEL_ALIGN_RIGHT  = 2
} IXLabelAlign;

// ---------------------------------------------------------------------------
//  IXLabel  —  texto com fundo/borda/gradiente/sprite totalmente customizaveis.
//
//  Fundo (ordem de prioridade):
//      bgTexture != NULL                ->  sprite (+ bgTextureTint)
//      bgGradTop/Bottom com alpha > 0   ->  gradiente
//      backgroundColor  com alpha > 0   ->  cor solida
//
//  Forma do fundo:
//      bgCornerRadius > 0  ->  cantos chanfrados (pill/badge)
//
//  Borda:
//      borderColor com alpha > 0  ->  contorno
//      strokeWidth                ->  espessura (default 1)
//
//  Texto:
//      drawShadow=1  ->  sombra 1px offset
//      wrapText=1    ->  quebra dentro de 'width'
//
//  Uso rapido:
//      IXLabel lbl;
//      Label_Init(&lbl, x, y, 160, 28, "Score: 0");
//
//  Badge/pill:
//      Label_SetBackground(&lbl, 0xCC1A3A5A, 8);
//      Label_SetBorder(&lbl, 0xFF3A8ACA);
//      Label_SetCornerRadius(&lbl, 14);    // pill = metade da altura
//
//  Gradiente:
//      Label_SetBackgroundGradient(&lbl, 0xDD1A3A5A, 0xDD0A1828, 8);
//
//  Sprite:
//      Label_SetBackgroundSprite(&lbl, myTex, 0xFFFFFFFF, 4);
// ---------------------------------------------------------------------------

typedef struct IXLabel
{
    int x, y, width, height;
    const char* text;

    // Fonte
    int fontSize;
    int fontBold;

    // Texto
    unsigned int textColor;
    unsigned int shadowColor;
    int          drawShadow;
    int          alignment;
    int          wrapText;

    // Fundo solido
    unsigned int backgroundColor;
    int          backgroundPadding;

    // Fundo gradiente
    unsigned int bgGradTop;     // 0x00 = desativado
    unsigned int bgGradBottom;

    // Fundo sprite
    IDirect3DTexture9* bgTexture;    // NULL = sem sprite
    unsigned int       bgTextureTint;

    // Borda
    unsigned int borderColor;
    int          strokeWidth;    // default 1

    // Forma do fundo
    int bgCornerRadius;          // 0 = quadrado, >0 = chanfrado
} IXLabel;

// --- Inicializacao ---
void Label_Init(IXLabel* label, int x, int y, int width, int height, const char* text);
void Label_SetText(IXLabel* label, const char* text);
void Label_SetBounds(IXLabel* label, int x, int y, int width, int height);

// --- Fonte ---
void Label_SetFontSize(IXLabel* label, int fontSize);
void Label_SetBold(IXLabel* label, int bold);

// --- Texto ---
void Label_SetAlignment(IXLabel* label, int alignment);
void Label_SetTextColor(IXLabel* label, unsigned int color);
void Label_EnableShadow(IXLabel* label, int enabled);
void Label_SetShadowColor(IXLabel* label, unsigned int color);
void Label_SetWrap(IXLabel* label, int wrap);

// --- Fundo solido ---
void Label_SetBackground(IXLabel* label, unsigned int color, int padding);

// --- Fundo gradiente ---
void Label_SetBackgroundGradient(IXLabel* label,
                                  unsigned int top, unsigned int bottom, int padding);

// --- Fundo sprite ---
void Label_SetBackgroundSprite(IXLabel* label,
                                IDirect3DTexture9* texture, unsigned int tint, int padding);

// --- Borda ---
void Label_SetBorder(IXLabel* label, unsigned int color);
void Label_SetStrokeWidth(IXLabel* label, int width);

// --- Forma ---
void Label_SetCornerRadius(IXLabel* label, int radius);

// --- Leitura ---
int Label_IsInside(const IXLabel* label, int pointX, int pointY);

#ifdef __cplusplus
}
#endif

#endif // IX_LABEL_H
