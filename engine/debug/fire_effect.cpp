#include "fire_effect.h"
#include "../renderer.h"
#include "../effects/effect.h"
#include "../interface/button.h"
#include "../interface/slider.h"
#include "../interface/layout.h"
#include <cstdio>

// ---------------------------------------------------------------------------
//  UI widgets
// ---------------------------------------------------------------------------
static IXButton g_speedButton;
static IXSlider g_speedSlider;
static IXSlider g_windSlider;
static IXSlider g_intensitySlider;
static IXSlider g_decaySlider;
static IXSlider g_blurSlider;
static IXSlider g_bufferWidthSlider;
static IXSlider g_bufferHeightSlider;
static IXSlider g_viewWidthSlider;
static IXSlider g_viewHeightSlider;

static const IXButton* g_buttons[1];
static const IXSlider* g_sliders[9];

static char g_statusLine[256]       = "Ready";
static char g_speedButtonLabel[64]  = "Preset: 33 ms";
static char g_speedLabel[64]        = "Simulation delay";
static char g_windLabel[64]         = "Wind direction / force";
static char g_intensityLabel[64]    = "Base intensity";
static char g_decayLabel[64]        = "Flame decay";
static char g_blurLabel[64]         = "Blur strength";
static char g_bufferWidthLabel[64]  = "Fire buffer width";
static char g_bufferHeightLabel[64] = "Fire buffer height";
static char g_viewWidthLabel[64]    = "Viewport width";
static char g_viewHeightLabel[64]   = "Viewport height";

// ---------------------------------------------------------------------------
//  Fire viewport  (owned here, was previously stored in the renderer)
// ---------------------------------------------------------------------------
static int g_vpX = 280;
static int g_vpY = 72;
static int g_vpW = 640;
static int g_vpH = 480;

static void Viewport_Clamp()
{
    int clientW = 0;
    int clientH = 0;
    Renderer_GetClientSize(&clientW, &clientH);
    if (clientW < 32) clientW = 32;
    if (clientH < 32) clientH = 32;

    if (g_vpW < 32) g_vpW = 32;
    if (g_vpH < 32) g_vpH = 32;
    if (g_vpW > clientW) g_vpW = clientW;
    if (g_vpH > clientH) g_vpH = clientH;
    if (g_vpX < 0) g_vpX = 0;
    if (g_vpY < 0) g_vpY = 0;
    if (g_vpX + g_vpW > clientW) g_vpX = clientW - g_vpW;
    if (g_vpY + g_vpH > clientH) g_vpY = clientH - g_vpH;
    if (g_vpX < 0) g_vpX = 0;
    if (g_vpY < 0) g_vpY = 0;
}

static void Viewport_Set(int x, int y, int w, int h)
{
    if (w > 0) g_vpW = w;
    if (h > 0) g_vpH = h;
    if (x >= 0) g_vpX = x;
    if (y >= 0) g_vpY = y;
    Viewport_Clamp();
}

static int Viewport_PointInside(int x, int y)
{
    return x >= g_vpX && x < (g_vpX + g_vpW) &&
           y >= g_vpY && y < (g_vpY + g_vpH);
}

// ---------------------------------------------------------------------------
//  Drag state
// ---------------------------------------------------------------------------
static int g_draggingFire    = 0;
static int g_fireDragOffsetX = 0;
static int g_fireDragOffsetY = 0;
static int g_returnToMenu    = 0;

// ---------------------------------------------------------------------------
//  Simulation accumulator
// ---------------------------------------------------------------------------
static double g_simAccumulatorMs = 0.0;

// ---------------------------------------------------------------------------
//  Layout cache
// ---------------------------------------------------------------------------
struct FireLayoutCache { int valid, x, y, width, height; };
static FireLayoutCache g_layoutCache = {0, 0, 0, 0, 0};

static void RefreshLayout()
{
    IXLayout layout;
    int x, y, width, height;
    Renderer_GetOverlayContentRect(&x, &y, &width, &height);
    g_layoutCache = { 1, x, y, width, height };

    Layout_BeginVertical(&layout, x, y + 82, width, height - 82, 8);
    Layout_Next(&layout, 34, &x, &y, &width, &height); Button_SetBounds(&g_speedButton, x, y, width, height);
    Layout_Skip(&layout, 6);
    Layout_Next(&layout, 34, &x, &y, &width, &height); Slider_SetBounds(&g_speedSlider,       x, y, width - 56, height);
    Layout_Next(&layout, 34, &x, &y, &width, &height); Slider_SetBounds(&g_windSlider,         x, y, width - 56, height);
    Layout_Next(&layout, 34, &x, &y, &width, &height); Slider_SetBounds(&g_intensitySlider,    x, y, width - 56, height);
    Layout_Next(&layout, 34, &x, &y, &width, &height); Slider_SetBounds(&g_decaySlider,        x, y, width - 56, height);
    Layout_Next(&layout, 34, &x, &y, &width, &height); Slider_SetBounds(&g_blurSlider,         x, y, width - 56, height);
    Layout_Next(&layout, 34, &x, &y, &width, &height); Slider_SetBounds(&g_bufferWidthSlider,  x, y, width - 56, height);
    Layout_Next(&layout, 34, &x, &y, &width, &height); Slider_SetBounds(&g_bufferHeightSlider, x, y, width - 56, height);
    Layout_Next(&layout, 34, &x, &y, &width, &height); Slider_SetBounds(&g_viewWidthSlider,    x, y, width - 56, height);
    Layout_Next(&layout, 34, &x, &y, &width, &height); Slider_SetBounds(&g_viewHeightSlider,   x, y, width - 56, height);
}

static void EnsureLayout()
{
    int x, y, w, h;
    Renderer_GetOverlayContentRect(&x, &y, &w, &h);
    if (!g_layoutCache.valid ||
        g_layoutCache.x != x || g_layoutCache.y != y ||
        g_layoutCache.width != w || g_layoutCache.height != h)
        RefreshLayout();
}

// ---------------------------------------------------------------------------
//  Internal helpers
// ---------------------------------------------------------------------------
static void CycleSpeedPreset()
{
    int speed = Effect_GetFireSpeed();
    speed = (speed == 33) ? 16 : (speed == 16) ? 50 : 33;
    Effect_SetFireSpeed(speed);
    Slider_SetValue(&g_speedSlider, speed);
}

static void RebuildFireFromCurrentState()
{
    FireSettings s = Effect_GetFireSettings();
    s.width        = Slider_GetValue(&g_bufferWidthSlider);
    s.height       = Slider_GetValue(&g_bufferHeightSlider);
    s.windStrength  = Slider_GetValue(&g_windSlider);
    s.baseIntensity = Slider_GetValue(&g_intensitySlider);
    s.maxDecay      = Slider_GetValue(&g_decaySlider);
    Effect_InitializeFire(s);
    Effect_SetFireSpeed(Slider_GetValue(&g_speedSlider));
    Effect_SetFireBlur(Slider_GetValue(&g_blurSlider));
}

static void ApplySliderState(int rebuildBuffer)
{
    Effect_SetFireSpeed(Slider_GetValue(&g_speedSlider));
    Effect_SetFireWind(Slider_GetValue(&g_windSlider));
    Effect_SetFireBaseIntensity(Slider_GetValue(&g_intensitySlider));
    Effect_SetFireDecay(Slider_GetValue(&g_decaySlider));
    Effect_SetFireBlur(Slider_GetValue(&g_blurSlider));
    Viewport_Set(-1, -1,
        Slider_GetValue(&g_viewWidthSlider),
        Slider_GetValue(&g_viewHeightSlider));
    if (rebuildBuffer)
        RebuildFireFromCurrentState();
}

static void RefreshViewportOnly()
{
    Viewport_Set(g_vpX, g_vpY,
        Slider_GetValue(&g_viewWidthSlider),
        Slider_GetValue(&g_viewHeightSlider));
}

static void UpdateOverlayText()
{
    std::snprintf(g_statusLine, sizeof(g_statusLine),
        "Delay %d ms | Wind %d | Intensity %d | Decay %d | Blur %d | Buffer %dx%d | View %dx%d @ %d,%d",
        Effect_GetFireSpeed(), Effect_GetFireWind(),
        Effect_GetFireBaseIntensity(), Effect_GetFireDecay(), Effect_GetFireBlur(),
        Effect_GetFireWidth(), Effect_GetFireHeight(),
        g_vpW, g_vpH, g_vpX, g_vpY);

    std::snprintf(g_speedButtonLabel, sizeof(g_speedButtonLabel), "Preset: %d ms", Effect_GetFireSpeed());
    Button_SetText(&g_speedButton, g_speedButtonLabel);
    Slider_SetValue(&g_speedSlider,       Effect_GetFireSpeed());
    Slider_SetValue(&g_windSlider,         Effect_GetFireWind());
    Slider_SetValue(&g_intensitySlider,    Effect_GetFireBaseIntensity());
    Slider_SetValue(&g_decaySlider,        Effect_GetFireDecay());
    Slider_SetValue(&g_blurSlider,         Effect_GetFireBlur());
    Slider_SetValue(&g_bufferWidthSlider,  Effect_GetFireWidth());
    Slider_SetValue(&g_bufferHeightSlider, Effect_GetFireHeight());
    Slider_SetValue(&g_viewWidthSlider,    g_vpW);
    Slider_SetValue(&g_viewHeightSlider,   g_vpH);
}

// ---------------------------------------------------------------------------
//  Public API
// ---------------------------------------------------------------------------
void DebugFireEffect_Initialize()
{
    FireSettings s;
    s.width        = 160;
    s.height       = 120;
    s.paletteSize  = 37;
    s.maxDecay     = 4;
    s.windStrength  = 0;
    s.baseIntensity = 36;
    Effect_InitializeFire(s);
    Effect_SetFireSpeed(33);
    Effect_SetFireBlur(1);

    Button_Init(&g_speedButton, 18, 134, 150, 34, g_speedButtonLabel);
    Slider_Init(&g_speedSlider,       18, 192, 220, 34,   1,  60, Effect_GetFireSpeed(),        g_speedLabel);
    Slider_Init(&g_windSlider,         18, 236, 220, 34,  -6,   6, Effect_GetFireWind(),          g_windLabel);
    Slider_Init(&g_intensitySlider,    18, 280, 220, 34,   1,  36, Effect_GetFireBaseIntensity(), g_intensityLabel);
    Slider_Init(&g_decaySlider,        18, 324, 220, 34,   1,   8, Effect_GetFireDecay(),         g_decayLabel);
    Slider_Init(&g_blurSlider,         18, 368, 220, 34,   0,   3, Effect_GetFireBlur(),          g_blurLabel);
    Slider_Init(&g_bufferWidthSlider,  18, 412, 220, 34,  64, 320, s.width,                       g_bufferWidthLabel);
    Slider_Init(&g_bufferHeightSlider, 18, 456, 220, 34,  48, 240, s.height,                      g_bufferHeightLabel);
    Slider_Init(&g_viewWidthSlider,    18, 500, 220, 34, 160, 820, 640,                            g_viewWidthLabel);
    Slider_Init(&g_viewHeightSlider,   18, 544, 220, 34, 120, 560, 480,                            g_viewHeightLabel);

    g_buttons[0] = &g_speedButton;
    g_sliders[0] = &g_speedSlider;
    g_sliders[1] = &g_windSlider;
    g_sliders[2] = &g_intensitySlider;
    g_sliders[3] = &g_decaySlider;
    g_sliders[4] = &g_blurSlider;
    g_sliders[5] = &g_bufferWidthSlider;
    g_sliders[6] = &g_bufferHeightSlider;
    g_sliders[7] = &g_viewWidthSlider;
    g_sliders[8] = &g_viewHeightSlider;

    g_draggingFire    = 0;
    g_fireDragOffsetX = 0;
    g_fireDragOffsetY = 0;
    g_returnToMenu    = 0;
    g_simAccumulatorMs = 0.0;

    // Reset viewport to default position
    g_vpX = 280; g_vpY = 72; g_vpW = 640; g_vpH = 480;
    Viewport_Clamp();

    g_layoutCache.valid = 0;
    EnsureLayout();
    UpdateOverlayText();
}

void DebugFireEffect_InvalidateLayout()
{
    g_layoutCache.valid = 0;
}

void DebugFireEffect_GetViewport(int* x, int* y, int* w, int* h)
{
    if (x) *x = g_vpX;
    if (y) *y = g_vpY;
    if (w) *w = g_vpW;
    if (h) *h = g_vpH;
}

void DebugFireEffect_HandleMouseMove(int mouseX, int mouseY)
{
    if (g_draggingFire)
    {
        Viewport_Set(mouseX - g_fireDragOffsetX,
                     mouseY - g_fireDragOffsetY,
                     g_vpW, g_vpH);
        UpdateOverlayText();
    }
}

void DebugFireEffect_BeginMouseDrag(int mouseX, int mouseY)
{
    EnsureLayout();
    if (!DebugFireEffect_IsPointOverUI(mouseX, mouseY) &&
        Viewport_PointInside(mouseX, mouseY))
    {
        g_draggingFire    = 1;
        g_fireDragOffsetX = mouseX - g_vpX;
        g_fireDragOffsetY = mouseY - g_vpY;
    }
}

void DebugFireEffect_EndMouseDrag()
{
    g_draggingFire = 0;
}

void DebugFireEffect_HandleKeyDown(WPARAM key)
{
    if (key == VK_SPACE)
    {
        CycleSpeedPreset();
        UpdateOverlayText();
    }
    else if (key == 'R')
    {
        Effect_ResetFire();
        UpdateOverlayText();
    }
    else if (key == VK_ESCAPE)
    {
        g_returnToMenu = 1;
    }
}

void DebugFireEffect_UpdateUI(int mouseX, int mouseY, int mouseDown, int mouseReleased)
{
    int changed      = 0;
    int rebuildBuffer = 0;
    EnsureLayout();

    if (!g_draggingFire)
    {
        if (Button_Handle(&g_speedButton, mouseX, mouseY, mouseDown, mouseReleased))
        {
            CycleSpeedPreset();
            changed = 1;
        }
        changed |= Slider_Handle(&g_speedSlider,    mouseX, mouseY, mouseDown);
        changed |= Slider_Handle(&g_windSlider,      mouseX, mouseY, mouseDown);
        changed |= Slider_Handle(&g_intensitySlider, mouseX, mouseY, mouseDown);
        changed |= Slider_Handle(&g_decaySlider,     mouseX, mouseY, mouseDown);
        changed |= Slider_Handle(&g_blurSlider,      mouseX, mouseY, mouseDown);
        if (Slider_Handle(&g_bufferWidthSlider,  mouseX, mouseY, mouseDown)) { changed = 1; rebuildBuffer = 1; }
        if (Slider_Handle(&g_bufferHeightSlider, mouseX, mouseY, mouseDown)) { changed = 1; rebuildBuffer = 1; }
        if (Slider_Handle(&g_viewWidthSlider,    mouseX, mouseY, mouseDown))   changed = 1;
        if (Slider_Handle(&g_viewHeightSlider,   mouseX, mouseY, mouseDown))   changed = 1;
    }

    if (changed)
    {
        RefreshViewportOnly();
        ApplySliderState(rebuildBuffer);
        UpdateOverlayText();
    }
}

void DebugFireEffect_ApplyRendererState(float renderFPS, float frameMs, float simUPS)
{
    EnsureLayout();
    Renderer_SetOverlayText("InteractiveX r0.2", "Fire Effect debug", g_statusLine);
    Renderer_SetHelpText("Drag fire with mouse | SPACE preset | R reset | ESC menu");
    Renderer_SetDisplayOptions(1, 1, 1);
    Renderer_SetControlButton(NULL);
    Renderer_SetControlButtons(g_buttons, 1);
    Renderer_SetControlSliders(g_sliders, 9);
    Renderer_SetOverlayMetrics(renderFPS, frameMs, simUPS);
}

int DebugFireEffect_IsPointOverUI(int x, int y)
{
    for (int i = 0; i < 1; i++) if (Button_IsInside(g_buttons[i], x, y)) return 1;
    for (int i = 0; i < 9; i++) if (Slider_IsInside(g_sliders[i], x, y)) return 1;
    return 0;
}

int DebugFireEffect_IsDraggingFire() { return g_draggingFire; }

void DebugFireEffect_AccumulateSimulation(double deltaMs, unsigned int* simStepsOut)
{
    double simStepMs = (double)Effect_GetFireSpeed();
    if (simStepMs < 1.0) simStepMs = 1.0;
    g_simAccumulatorMs += deltaMs;
    while (g_simAccumulatorMs >= simStepMs)
    {
        Effect_UpdateFire();
        g_simAccumulatorMs -= simStepMs;
        if (simStepsOut) (*simStepsOut)++;
    }
}

const unsigned int* DebugFireEffect_GetPixelBuffer() { return Effect_GetFireBuffer(); }
int                 DebugFireEffect_GetBufferWidth()  { return Effect_GetFireWidth();  }
int                 DebugFireEffect_GetBufferHeight() { return Effect_GetFireHeight(); }
int                 DebugFireEffect_ShouldReturnToMenu() { return g_returnToMenu; }
void                DebugFireEffect_ClearReturnToMenu()  { g_returnToMenu = 0; }
