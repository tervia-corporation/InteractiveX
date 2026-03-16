#ifndef IX_SLIDER_H
#define IX_SLIDER_H

#ifdef __cplusplus
extern "C" {
#endif

struct IDirect3DTexture9;

// ---------------------------------------------------------------------------
//  IXSlider  —  controle deslizante com aparencia totalmente customizavel.
//
//  Track:
//      trackRadius > 0  ->  ponta arredondada (pill-shape nos extremos)
//      trackInsetColor alpha > 0  ->  camada inset/sombra interna abaixo da track
//
//  Knob:
//      knobRadius > 0           ->  knob chanfrado
//      knobGradTop/Bottom != 0  ->  gradiente no knob
//      knobStrokeColor          ->  contorno do knob
//
//  Valores:
//      showValue  1 = mostra numero a direita, 0 = oculta
//      valueColor, valueFontSize, labelFontSize para customizacao
//
//  Sprite:
//      knobTexture != NULL  ->  desenha sprite como knob
//
//  Uso rapido (igual ao antigo):
//      IXSlider s;
//      Slider_Init(&s, x, y, w, h, 0, 100, 50, "Volume");
//
//  Uso CSS:
//      Slider_SetTrackStyle(&s, 0x88101820, 0xCC2A7ABA, 8, 4);
//      Slider_SetKnobStyle(&s, 0xFFE0E8F0, 0xFF8AB8D8, 14, 18, 4);
//      Slider_SetKnobStroke(&s, 0xFF3A8ACA, 1);
//      Slider_SetValueStyle(&s, 0xFFFFD27F, 16);
// ---------------------------------------------------------------------------

typedef struct IXSlider
{
    int x, y, width, height;

    int minValue, maxValue, value;
    const char* label;

    // Track
    unsigned int trackColor;
    unsigned int fillColor;
    int          trackHeight;   // altura da barra (default 6)
    int          trackRadius;   // arredondamento das pontas (default 0)
    unsigned int trackInsetColor; // camada de sombra interna (0x00 = off)

    // Knob
    unsigned int knobColor;
    unsigned int knobGradTop;    // 0x00 = desativado
    unsigned int knobGradBottom;
    unsigned int knobStrokeColor; // 0x00 = sem contorno
    int          knobWidth;      // default 8
    int          knobHeight;     // default 14
    int          knobRadius;     // chanfro (default 0)
    IDirect3DTexture9* knobTexture; // NULL = usa cor

    // Texto
    unsigned int textColor;
    unsigned int valueColor;     // cor do numero (default 0xFFFFD27F)
    int          labelFontSize;  // 0 = padrao
    int          valueFontSize;  // 0 = padrao
    int          showValue;      // 1 = exibe numero (default 1)

    // Runtime
    int hovered;
    int dragging;
} IXSlider;

// --- Inicializacao ---
void Slider_Init(IXSlider* slider, int x, int y, int width, int height,
                 int minValue, int maxValue, int value, const char* label);
void Slider_SetLabel(IXSlider* slider, const char* label);
void Slider_SetBounds(IXSlider* slider, int x, int y, int width, int height);
void Slider_SetValue(IXSlider* slider, int value);
int  Slider_GetValue(const IXSlider* slider);
int  Slider_IsInside(const IXSlider* slider, int mouseX, int mouseY);
int  Slider_Handle(IXSlider* slider, int mouseX, int mouseY, int mouseDown);

// --- Track ---
// trackColor: fundo, fillColor: preenchido, height: altura px, radius: arredondamento
void Slider_SetTrackStyle(IXSlider* slider,
                           unsigned int trackColor, unsigned int fillColor,
                           int height, int radius);
void Slider_SetTrackInset(IXSlider* slider, unsigned int insetColor);

// --- Knob ---
// color: cor solida, gradTop/Bottom: gradiente (0x00=off),
// w/h: dimensoes, radius: chanfro
void Slider_SetKnobStyle(IXSlider* slider,
                          unsigned int color,
                          unsigned int gradTop, unsigned int gradBottom,
                          int width, int height, int radius);
void Slider_SetKnobStroke(IXSlider* slider, unsigned int color, int strokeWidth);
void Slider_SetKnobSprite(IXSlider* slider, IDirect3DTexture9* texture);

// --- Texto e valores ---
void Slider_SetValueStyle(IXSlider* slider, unsigned int valueColor, int valueFontSize);
void Slider_SetLabelFontSize(IXSlider* slider, int fontSize);
void Slider_ShowValue(IXSlider* slider, int show);

#ifdef __cplusplus
}
#endif

#endif // IX_SLIDER_H
