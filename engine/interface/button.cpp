#include "button.h"
#include <string.h>

// ---------------------------------------------------------------------------
//  Helpers internos
// ---------------------------------------------------------------------------
static void StyleInit(IXButtonStyle* s,
                      unsigned int bg, unsigned int stroke, int sw, unsigned int text)
{
    s->bgColor        = bg;
    s->gradColorTop   = 0x00000000;
    s->gradColorBottom= 0x00000000;
    s->strokeColor    = stroke;
    s->strokeWidth    = sw;
    s->textColor      = text;
    s->imageTint      = 0xFFFFFFFF;
}

// ---------------------------------------------------------------------------
//  Inicializacao
// ---------------------------------------------------------------------------
void Button_Init(IXButton* b, int x, int y, int w, int h, const char* text)
{
    if (!b) return;
    memset(b, 0, sizeof(IXButton));

    b->x = x; b->y = y; b->width = w; b->height = h;
    b->text    = text;
    b->tooltip = NULL;

    // Fonte
    b->fontSize         = 0;
    b->fontBold         = 1;
    b->textAlignment    = 1;    // centro
    b->textOffsetPressX = 0;
    b->textOffsetPressY = 1;

    // Forma
    b->cornerRadius = 0;
    b->opacity      = 255;
    b->enabled      = 1;

    // Estilos
    StyleInit(&b->styleNormal,   0xCC1E1E1E, 0xCCFFFFFF, 1, 0xFFFFFFFF);
    StyleInit(&b->styleHover,    0xCC353535, 0xCCFFFFFF, 1, 0xFFFFFFFF);
    StyleInit(&b->stylePressed,  0xCC555555, 0xCCFFFFFF, 1, 0xFFFFFFFF);
    StyleInit(&b->styleDisabled, 0x881E1E1E, 0x44FFFFFF, 1, 0x88AAAAAA);

    // Sprite
    b->spriteTexture = NULL;
}

// ---------------------------------------------------------------------------
//  Conteudo
// ---------------------------------------------------------------------------
void Button_SetText(IXButton* b, const char* text)       { if (b) b->text    = text;    }
void Button_SetTooltip(IXButton* b, const char* tooltip) { if (b) b->tooltip = tooltip; }

void Button_SetBounds(IXButton* b, int x, int y, int w, int h)
{
    if (!b) return;
    b->x = x; b->y = y; b->width = w; b->height = h;
}

// ---------------------------------------------------------------------------
//  Forma e aparencia global
// ---------------------------------------------------------------------------
void Button_SetCornerRadius(IXButton* b, int r)   { if (b) b->cornerRadius = (r >= 0) ? r : 0; }
void Button_SetOpacity(IXButton* b, int op)        { if (b) b->opacity = (op<0)?0:(op>255)?255:op; }
void Button_SetEnabled(IXButton* b, int en)        { if (b) b->enabled = en ? 1 : 0; }

// ---------------------------------------------------------------------------
//  Fonte e texto
// ---------------------------------------------------------------------------
void Button_SetFont(IXButton* b, int sz, int bold)
{
    if (!b) return;
    b->fontSize  = (sz > 0) ? sz : 0;
    b->fontBold  = bold ? 1 : 0;
}

void Button_SetTextAlignment(IXButton* b, int al)
{
    if (!b) return;
    if (al < 0) al = 0;
    if (al > 2) al = 2;
    b->textAlignment = al;
}

void Button_SetTextPressOffset(IXButton* b, int dx, int dy)
{
    if (!b) return;
    b->textOffsetPressX = dx;
    b->textOffsetPressY = dy;
}

// ---------------------------------------------------------------------------
//  Atalhos de estilo
// ---------------------------------------------------------------------------
void Button_SetColors(IXButton* b,
                      unsigned int normal, unsigned int hover, unsigned int pressed)
{
    if (!b) return;
    b->styleNormal.bgColor  = normal;
    b->styleHover.bgColor   = hover;
    b->stylePressed.bgColor = pressed;
    // Limpa gradientes para evitar conflito
    b->styleNormal.gradColorTop = b->styleNormal.gradColorBottom = 0;
    b->styleHover.gradColorTop  = b->styleHover.gradColorBottom  = 0;
    b->stylePressed.gradColorTop= b->stylePressed.gradColorBottom= 0;
}

void Button_SetGradients(IXButton* b,
                         unsigned int topN,  unsigned int botN,
                         unsigned int topH,  unsigned int botH,
                         unsigned int topP,  unsigned int botP)
{
    if (!b) return;
    b->styleNormal.gradColorTop   = topN; b->styleNormal.gradColorBottom  = botN;
    b->styleHover.gradColorTop    = topH; b->styleHover.gradColorBottom   = botH;
    b->stylePressed.gradColorTop  = topP; b->stylePressed.gradColorBottom = botP;
}

void Button_SetStroke(IXButton* b, unsigned int color, int width)
{
    if (!b) return;
    b->styleNormal.strokeColor  = b->styleHover.strokeColor  = b->stylePressed.strokeColor  = color;
    b->styleNormal.strokeWidth  = b->styleHover.strokeWidth  = b->stylePressed.strokeWidth  = width;
}

void Button_SetTextColors(IXButton* b,
                          unsigned int normal, unsigned int hover, unsigned int pressed)
{
    if (!b) return;
    b->styleNormal.textColor  = normal;
    b->styleHover.textColor   = hover;
    b->stylePressed.textColor = pressed;
}

IXButtonStyle* Button_GetStyle(IXButton* b, int state)
{
    if (!b) return NULL;
    switch (state) {
        case IX_BTN_HOVER:    return &b->styleHover;
        case IX_BTN_PRESSED:  return &b->stylePressed;
        case IX_BTN_DISABLED: return &b->styleDisabled;
        default:              return &b->styleNormal;
    }
}

// ---------------------------------------------------------------------------
//  Sprite
// ---------------------------------------------------------------------------
void Button_SetSprite(IXButton* b, IDirect3DTexture9* tex)
{
    if (!b) return;
    b->spriteTexture = tex;
}

void Button_SetSpriteFrame(IXButton* b, int state, int sx, int sy, int sw, int sh)
{
    if (!b) return;
    switch (state) {
        case IX_BTN_NORMAL:
            b->spriteNormalX=sx; b->spriteNormalY=sy; b->spriteNormalW=sw; b->spriteNormalH=sh; break;
        case IX_BTN_HOVER:
            b->spriteHoverX=sx;  b->spriteHoverY=sy;  b->spriteHoverW=sw;  b->spriteHoverH=sh;  break;
        case IX_BTN_PRESSED:
            b->spritePressedX=sx; b->spritePressedY=sy; b->spritePressedW=sw; b->spritePressedH=sh; break;
        case IX_BTN_DISABLED:
            b->spriteDisabledX=sx; b->spriteDisabledY=sy; b->spriteDisabledW=sw; b->spriteDisabledH=sh; break;
    }
}

void Button_SetSpriteStrip(IXButton* b, IDirect3DTexture9* tex, int fw, int fh)
{
    if (!b) return;
    b->spriteTexture = tex;
    Button_SetSpriteFrame(b, IX_BTN_NORMAL,   0,    0, fw, fh);
    Button_SetSpriteFrame(b, IX_BTN_HOVER,    fw,   0, fw, fh);
    Button_SetSpriteFrame(b, IX_BTN_PRESSED,  fw*2, 0, fw, fh);
    Button_SetSpriteFrame(b, IX_BTN_DISABLED, fw*3, 0, fw, fh);
}

// ---------------------------------------------------------------------------
//  Input e leitura
// ---------------------------------------------------------------------------
int Button_IsInside(const IXButton* b, int mx, int my)
{
    if (!b) return 0;
    return (mx >= b->x && mx < b->x + b->width &&
            my >= b->y && my < b->y + b->height) ? 1 : 0;
}

int Button_Handle(IXButton* b, int mx, int my, int down, int released)
{
    if (!b || !b->enabled) return 0;

    int inside  = Button_IsInside(b, mx, my);
    b->hovered  = inside;

    if (inside && down)   b->pressed = 1;

    if (b->pressed && released)
    {
        b->pressed = 0;
        if (inside) return 1;
    }

    if (!down && !inside) b->pressed = 0;
    return 0;
}

const char* Button_GetTooltip(const IXButton* b)
{
    if (!b || !b->hovered) return NULL;
    return b->tooltip;
}

const IXButtonStyle* Button_GetCurrentStyle(const IXButton* b)
{
    if (!b) return NULL;
    if (!b->enabled)  return &b->styleDisabled;
    if (b->pressed)   return &b->stylePressed;
    if (b->hovered)   return &b->styleHover;
    return &b->styleNormal;
}
