#ifndef IX_BUTTON_H
#define IX_BUTTON_H

#ifdef __cplusplus
extern "C" {
#endif

struct IDirect3DTexture9;

// ---------------------------------------------------------------------------
//  IXButtonStyle  —  visual de um unico estado
//
//  Fundo:
//      gradColorTop/Bottom != 0x00  ->  gradiente top-to-bottom
//      caso contrario              ->  cor solida (bgColor)
//
//  Stroke:
//      strokeColor alpha > 0 E strokeWidth > 0  ->  borda desenhada
//
//  Sprite:
//      imageTint é o multiplicador ARGB aplicado sobre a textura.
//      0xFFFFFFFF = sem alteracao (padrao).
// ---------------------------------------------------------------------------
typedef struct IXButtonStyle
{
    unsigned int bgColor;           // fundo solido
    unsigned int gradColorTop;      // gradiente: 0x00... = desativado
    unsigned int gradColorBottom;
    unsigned int strokeColor;       // borda: 0x00... = desativada
    int          strokeWidth;       // espessura em pixels (padrao 1)
    unsigned int textColor;
    unsigned int imageTint;         // multiplicador de cor para sprite (0xFFFFFFFF = neutro)
} IXButtonStyle;

// ---------------------------------------------------------------------------
//  IXButton
//
//  3 estados interativos (normal, hover, pressed) + 1 estado disabled.
//  Design 100% customizavel por estado: gradiente, stroke, cornerRadius, opacity.
//  Suporte a spritesheet com mapeamento livre de cada estado.
//  Bônus: tooltip e disabled state.
//
//  Uso rapido (somente cor solida, igual ao antigo):
//      IXButton btn;
//      Button_Init(&btn, x, y, w, h, "Click me");
//      Button_SetColors(&btn, 0xCC1E2840, 0xCC284060, 0xCC345880);
//
//  Uso gradiente:
//      Button_SetGradients(&btn,
//          0xFF1a2a3a, 0xFF0a1a2a,   // normal  top/bot
//          0xFF2a4a6a, 0xFF1a3a5a,   // hover   top/bot
//          0xFF3a6a9a, 0xFF2a5a8a);  // pressed top/bot
//
//  Uso sprite:
//      Button_SetSprite(&btn, myTexture);
//      Button_SetSpriteFrame(&btn, IX_BTN_NORMAL,   0,   0, 64, 32);
//      Button_SetSpriteFrame(&btn, IX_BTN_HOVER,   64,   0, 64, 32);
//      Button_SetSpriteFrame(&btn, IX_BTN_PRESSED, 128,  0, 64, 32);
//
//  Ou com strip horizontal uniforme:
//      Button_SetSpriteStrip(&btn, myTexture, 64, 32);
//      // mapeia: normal=frame0, hover=frame1, pressed=frame2, disabled=frame3
// ---------------------------------------------------------------------------

#define IX_BTN_NORMAL   0
#define IX_BTN_HOVER    1
#define IX_BTN_PRESSED  2
#define IX_BTN_DISABLED 3

typedef struct IXButton
{
    // Bounds
    int x, y, width, height;

    // Conteudo
    const char* text;
    const char* tooltip;    // retornado por Button_GetTooltip() quando hovered

    // Fonte
    int fontSize;           // 0 = padrao da engine (18px)
    int fontBold;           // 1 = negrito
    int textAlignment;      // 0=esquerda 1=centro 2=direita
    int textOffsetPressX;   // deslocamento do texto quando pressed
    int textOffsetPressY;   // (padrao: 0, 1 para sensacao tatil)

    // Forma
    int cornerRadius;       // 0 = quadrado, >0 = cantos chanfrados
    int opacity;            // 0-255: multiplicador global de alpha

    // Estilos por estado
    IXButtonStyle styleNormal;
    IXButtonStyle styleHover;
    IXButtonStyle stylePressed;
    IXButtonStyle styleDisabled;

    // Sprite (NULL = usa estilos de cor)
    IDirect3DTexture9* spriteTexture;
    int spriteNormalX,   spriteNormalY,   spriteNormalW,   spriteNormalH;
    int spriteHoverX,    spriteHoverY,    spriteHoverW,    spriteHoverH;
    int spritePressedX,  spritePressedY,  spritePressedW,  spritePressedH;
    int spriteDisabledX, spriteDisabledY, spriteDisabledW, spriteDisabledH;

    // Estado runtime (gerenciado por Button_Handle)
    int hovered;
    int pressed;
    int enabled;    // 0 = desabilitado (usa styleDisabled, nao responde a input)
} IXButton;

// ---------------------------------------------------------------------------
//  Inicializacao
// ---------------------------------------------------------------------------
// Defaults seguros: cor cinza escuro, stroke branco, texto branco,
// cornerRadius=0, opacity=255, enabled=1, fontBold=1, textAlignment=centro.
void Button_Init(IXButton* button, int x, int y, int width, int height, const char* text);

// ---------------------------------------------------------------------------
//  Conteudo
// ---------------------------------------------------------------------------
void Button_SetText(IXButton* button, const char* text);
void Button_SetTooltip(IXButton* button, const char* tooltip);
void Button_SetBounds(IXButton* button, int x, int y, int width, int height);

// ---------------------------------------------------------------------------
//  Forma e aparencia global
// ---------------------------------------------------------------------------
void Button_SetCornerRadius(IXButton* button, int radius);
void Button_SetOpacity(IXButton* button, int opacity);          // 0-255
void Button_SetEnabled(IXButton* button, int enabled);

// ---------------------------------------------------------------------------
//  Fonte e texto
// ---------------------------------------------------------------------------
void Button_SetFont(IXButton* button, int fontSize, int bold);
void Button_SetTextAlignment(IXButton* button, int alignment);  // IX_LABEL_ALIGN_*
void Button_SetTextPressOffset(IXButton* button, int dx, int dy);

// ---------------------------------------------------------------------------
//  Atalhos de estilo (definem os 3 estados ativos de uma vez)
// ---------------------------------------------------------------------------
// Cor solida de fundo para normal/hover/pressed
void Button_SetColors(IXButton* button,
                      unsigned int normal, unsigned int hover, unsigned int pressed);

// Gradiente top/bottom para normal/hover/pressed
void Button_SetGradients(IXButton* button,
                         unsigned int topNormal,  unsigned int botNormal,
                         unsigned int topHover,   unsigned int botHover,
                         unsigned int topPressed, unsigned int botPressed);

// Stroke (borda) para os 3 estados ativos (mesma cor e espessura)
void Button_SetStroke(IXButton* button, unsigned int color, int width);

// Cor do texto para normal/hover/pressed
void Button_SetTextColors(IXButton* button,
                          unsigned int normal, unsigned int hover, unsigned int pressed);

// Acessa e edita um estado especifico diretamente
IXButtonStyle* Button_GetStyle(IXButton* button, int state);  // IX_BTN_*

// ---------------------------------------------------------------------------
//  Sprite
// ---------------------------------------------------------------------------
// Define textura (NULL desativa sprite mode)
void Button_SetSprite(IXButton* button, IDirect3DTexture9* texture);

// Define a regiao da textura para um estado especifico
void Button_SetSpriteFrame(IXButton* button, int state,
                            int srcX, int srcY, int srcW, int srcH);

// Atalho: strip horizontal com frameW x frameH por frame
// Mapeia: normal=0, hover=1, pressed=2, disabled=3
void Button_SetSpriteStrip(IXButton* button, IDirect3DTexture9* texture,
                            int frameW, int frameH);

// ---------------------------------------------------------------------------
//  Input e leitura
// ---------------------------------------------------------------------------
int               Button_IsInside(const IXButton* button, int mouseX, int mouseY);
int               Button_Handle(IXButton* button, int mouseX, int mouseY,
                                 int mouseDown, int mouseReleased);
const char*       Button_GetTooltip(const IXButton* button);   // NULL se nao hovered
const IXButtonStyle* Button_GetCurrentStyle(const IXButton* button);

#ifdef __cplusplus
}
#endif

#endif // IX_BUTTON_H
