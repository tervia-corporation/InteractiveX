#include "slider.h"
#include <string.h>

static int SliderClamp(const IXSlider* s, int v)
{
    if (!s) return v;
    if (v < s->minValue) v = s->minValue;
    if (v > s->maxValue) v = s->maxValue;
    return v;
}

static int SliderTrackWidth(const IXSlider* s)
{
    if (!s->showValue) return s->width;
    const int valueArea = 48;
    const int gap       = 6;
    int t = s->width - valueArea - gap;
    return (t < 20) ? 20 : t;
}

void Slider_Init(IXSlider* s, int x, int y, int w, int h,
                 int mn, int mx, int val, const char* label)
{
    if (!s) return;
    memset(s, 0, sizeof(IXSlider));
    s->x = x; s->y = y; s->width = w; s->height = h;
    s->minValue = mn;
    s->maxValue = (mx <= mn) ? mn+1 : mx;
    s->value    = SliderClamp(s, val);
    s->label    = label;

    // Track
    s->trackColor      = 0xAA101820;
    s->fillColor       = 0xCC2A7ABA;
    s->trackHeight     = 6;
    s->trackRadius     = 3;
    s->trackInsetColor = 0x44000000;

    // Knob
    s->knobColor      = 0xFFDCECF8;
    s->knobGradTop    = 0xFFF0F8FF;
    s->knobGradBottom = 0xFFB0C8DC;
    s->knobStrokeColor= 0xFF3A8ACA;
    s->knobWidth      = 10;
    s->knobHeight     = 18;
    s->knobRadius     = 3;
    s->knobTexture    = NULL;

    // Texto
    s->textColor     = 0xFFCCCCCC;
    s->valueColor    = 0xFFFFD27F;
    s->labelFontSize = 0;
    s->valueFontSize = 0;
    s->showValue     = 1;
}

void Slider_SetLabel(IXSlider* s, const char* label)  { if (s) s->label = label; }
void Slider_SetBounds(IXSlider* s, int x, int y, int w, int h)
{
    if (!s) return;
    s->x=x; s->y=y; s->width=w; s->height=h;
}
void Slider_SetValue(IXSlider* s, int v) { if (s) s->value = SliderClamp(s, v); }
int  Slider_GetValue(const IXSlider* s)  { return s ? s->value : 0; }

int Slider_IsInside(const IXSlider* s, int mx, int my)
{
    if (!s) return 0;
    return (mx >= s->x && mx < s->x+SliderTrackWidth(s) &&
            my >= s->y && my < s->y+s->height) ? 1 : 0;
}

static int SliderValueFromMouse(const IXSlider* s, int mx)
{
    int tw = SliderTrackWidth(s);
    int range = s->maxValue - s->minValue;
    if (range <= 0 || tw <= 0) return s->minValue;
    int rel = mx - s->x;
    if (rel < 0)  rel = 0;
    if (rel > tw) rel = tw;
    return SliderClamp(s, s->minValue + (rel * range) / tw);
}

int Slider_Handle(IXSlider* s, int mx, int my, int down)
{
    if (!s) return 0;
    s->hovered = Slider_IsInside(s, mx, my);
    int old = s->value;
    if (down && s->hovered && !s->dragging) s->dragging = 1;
    if (!down) s->dragging = 0;
    if (s->dragging) s->value = SliderValueFromMouse(s, mx);
    return s->value != old;
}

// --- Style setters ---
void Slider_SetTrackStyle(IXSlider* s, unsigned int tc, unsigned int fc, int h, int r)
{
    if (!s) return;
    s->trackColor  = tc; s->fillColor   = fc;
    s->trackHeight = (h > 0) ? h : 4;
    s->trackRadius = (r >= 0) ? r : 0;
}
void Slider_SetTrackInset(IXSlider* s, unsigned int c) { if (s) s->trackInsetColor = c; }

void Slider_SetKnobStyle(IXSlider* s,
                          unsigned int color,
                          unsigned int gradTop, unsigned int gradBottom,
                          int kw, int kh, int r)
{
    if (!s) return;
    s->knobColor      = color;
    s->knobGradTop    = gradTop;
    s->knobGradBottom = gradBottom;
    s->knobWidth      = (kw > 0) ? kw : 8;
    s->knobHeight     = (kh > 0) ? kh : 14;
    s->knobRadius     = (r >= 0) ? r : 0;
}
void Slider_SetKnobStroke(IXSlider* s, unsigned int c, int sw)
{
    if (!s) return;
    s->knobStrokeColor = c;
    // sw guardado implicitamente — GUI_DrawSlider usa knobRadius para stroke
    (void)sw;
}
void Slider_SetKnobSprite(IXSlider* s, IDirect3DTexture9* t) { if (s) s->knobTexture = t; }

void Slider_SetValueStyle(IXSlider* s, unsigned int c, int fs)
{
    if (!s) return;
    s->valueColor    = c;
    s->valueFontSize = (fs > 0) ? fs : 0;
}
void Slider_SetLabelFontSize(IXSlider* s, int fs) { if (s) s->labelFontSize = (fs>0)?fs:0; }
void Slider_ShowValue(IXSlider* s, int show)       { if (s) s->showValue = show ? 1 : 0; }
