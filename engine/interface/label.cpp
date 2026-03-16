#include "label.h"
#include <string.h>

void Label_Init(IXLabel* l, int x, int y, int w, int h, const char* text)
{
    if (!l) return;
    memset(l, 0, sizeof(IXLabel));
    l->x=x; l->y=y; l->width=w; l->height=h;
    l->text          = text;
    l->fontSize      = 0;
    l->fontBold      = 1;
    l->textColor     = 0xFFFFFFFF;
    l->shadowColor   = 0xCC000000;
    l->drawShadow    = 0;
    l->alignment     = IX_LABEL_ALIGN_LEFT;
    l->wrapText      = 0;
    l->backgroundColor   = 0x00000000;
    l->backgroundPadding = 4;
    l->bgGradTop     = 0x00000000;
    l->bgGradBottom  = 0x00000000;
    l->bgTexture     = NULL;
    l->bgTextureTint = 0xFFFFFFFF;
    l->borderColor   = 0x00000000;
    l->strokeWidth   = 1;
    l->bgCornerRadius= 0;
}

void Label_SetText(IXLabel* l, const char* t)     { if (l) l->text = t; }
void Label_SetBounds(IXLabel* l, int x, int y, int w, int h)
{
    if (!l) return;
    l->x=x; l->y=y; l->width=w; l->height=h;
}
void Label_SetFontSize(IXLabel* l, int fs)        { if (l) l->fontSize = (fs>0)?fs:0; }
void Label_SetBold(IXLabel* l, int bold)          { if (l) l->fontBold = bold?1:0; }
void Label_SetAlignment(IXLabel* l, int a)
{
    if (!l) return;
    if (a<0) a=0; if (a>2) a=2;
    l->alignment = a;
}
void Label_SetTextColor(IXLabel* l, unsigned int c)   { if (l) l->textColor = c; }
void Label_EnableShadow(IXLabel* l, int en)            { if (l) l->drawShadow = en?1:0; }
void Label_SetShadowColor(IXLabel* l, unsigned int c)  { if (l) l->shadowColor = c; }
void Label_SetWrap(IXLabel* l, int wrap)               { if (l) l->wrapText = wrap?1:0; }

void Label_SetBackground(IXLabel* l, unsigned int color, int padding)
{
    if (!l) return;
    l->backgroundColor   = color;
    l->backgroundPadding = (padding>=0) ? padding : 0;
    l->bgGradTop = l->bgGradBottom = 0;
    l->bgTexture = NULL;
}
void Label_SetBackgroundGradient(IXLabel* l, unsigned int top, unsigned int bottom, int padding)
{
    if (!l) return;
    l->bgGradTop    = top;
    l->bgGradBottom = bottom;
    l->backgroundPadding = (padding>=0) ? padding : 0;
    l->bgTexture = NULL;
}
void Label_SetBackgroundSprite(IXLabel* l, IDirect3DTexture9* tex, unsigned int tint, int padding)
{
    if (!l) return;
    l->bgTexture     = tex;
    l->bgTextureTint = tint;
    l->backgroundPadding = (padding>=0) ? padding : 0;
}
void Label_SetBorder(IXLabel* l, unsigned int c)   { if (l) l->borderColor = c; }
void Label_SetStrokeWidth(IXLabel* l, int w)       { if (l) l->strokeWidth = (w>=0)?w:1; }
void Label_SetCornerRadius(IXLabel* l, int r)      { if (l) l->bgCornerRadius = (r>=0)?r:0; }
int  Label_IsInside(const IXLabel* l, int px, int py)
{
    if (!l) return 0;
    return (px>=l->x && px<l->x+l->width && py>=l->y && py<l->y+l->height) ? 1 : 0;
}
