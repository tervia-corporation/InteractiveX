#include "panel.h"
#include <string.h>

void Panel_Init(IXPanel* p, int x, int y, int w, int h, const char* title)
{
    if (!p) return;
    memset(p, 0, sizeof(IXPanel));
    p->x = x; p->y = y; p->width = w; p->height = h;
    p->title          = title;
    p->titleColor     = 0xFFFFFFFF;
    p->titleFontSize  = 0;
    p->titleFontBold  = 1;
    p->titleAlignment = 0;
    p->padding        = 10;
    p->headerHeight   = 32;

    // Fundo corpo
    p->backgroundColor = 0xCC060C14;
    p->bgGradTop       = 0x00000000;
    p->bgGradBottom    = 0x00000000;
    p->bgTexture       = NULL;
    p->bgTextureTint   = 0xFFFFFFFF;

    // Header
    p->headerColor    = 0xFF0D1E30;
    p->headerGradTop  = 0xFF152840;
    p->headerGradBottom = 0xFF0A1828;

    // Borda
    p->borderColor  = 0xFF1A4A6A;
    p->strokeWidth  = 1;
    p->cornerRadius = 0;
}

void Panel_SetTitle(IXPanel* p, const char* title)      { if (p) p->title = title; }
void Panel_SetBounds(IXPanel* p, int x, int y, int w, int h)
{
    if (!p) return;
    p->x=x; p->y=y; p->width=w; p->height=h;
}
int Panel_IsInside(const IXPanel* p, int px, int py)
{
    if (!p) return 0;
    return (px>=p->x && px<p->x+p->width && py>=p->y && py<p->y+p->height) ? 1 : 0;
}

void Panel_SetBackground(IXPanel* p, unsigned int color)
{
    if (!p) return;
    p->backgroundColor = color;
    p->bgGradTop = p->bgGradBottom = 0;
}
void Panel_SetBodyGradient(IXPanel* p, unsigned int top, unsigned int bottom)
{
    if (!p) return;
    p->bgGradTop = top; p->bgGradBottom = bottom;
}
void Panel_SetBodySprite(IXPanel* p, IDirect3DTexture9* tex, unsigned int tint)
{
    if (!p) return;
    p->bgTexture = tex; p->bgTextureTint = tint;
}

void Panel_SetHeaderColor(IXPanel* p, unsigned int color)
{
    if (!p) return;
    p->headerColor = color;
    p->headerGradTop = p->headerGradBottom = 0;
}
void Panel_SetHeaderGradient(IXPanel* p, unsigned int top, unsigned int bottom)
{
    if (!p) return;
    p->headerGradTop = top; p->headerGradBottom = bottom;
}
void Panel_SetHeaderHeight(IXPanel* p, int h) { if (p && h >= 0) p->headerHeight = h; }

void Panel_SetTitleFont(IXPanel* p, int fontSize, int bold, int alignment)
{
    if (!p) return;
    p->titleFontSize  = (fontSize > 0) ? fontSize : 0;
    p->titleFontBold  = bold ? 1 : 0;
    if (alignment < 0) alignment = 0;
    if (alignment > 2) alignment = 2;
    p->titleAlignment = alignment;
}
void Panel_SetTitleColor(IXPanel* p, unsigned int color) { if (p) p->titleColor = color; }

void Panel_SetBorderStyle(IXPanel* p, unsigned int color, int sw)
{
    if (!p) return;
    p->borderColor = color;
    p->strokeWidth = (sw >= 0) ? sw : 1;
}
void Panel_SetCornerRadius(IXPanel* p, int r) { if (p) p->cornerRadius = (r >= 0) ? r : 0; }
