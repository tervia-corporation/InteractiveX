#include "transition_test.h"
#include "../renderer.h"
#include "../core/input.h"
#include "../core/scene.h"
#include "../core/transition.h"
#include "../interface/button.h"
#include "../interface/slider.h"
#include "../interface/layout.h"
#include <cstdio>

// ---------------------------------------------------------------------------
//  Transition Test
//
//  Demonstra os tres tipos de transicao com duracao ajustavel.
//
//  Botoes:
//      Fade Black   -> transicao fade para preto, volta ao menu
//      Fade White   -> transicao fade para branco, volta ao menu
//      Wipe H       -> wipe horizontal, volta ao menu
//      Wipe V       -> wipe vertical, volta ao menu
//
//  Slider:
//      Duration     -> duracao da transicao (100ms a 1500ms)
//
//  Teclas:
//      ESC -> volta ao menu instantaneamente (sem transicao)
// ---------------------------------------------------------------------------

namespace
{
    static IXButton g_btnFadeBlack;
    static IXButton g_btnFadeWhite;
    static IXButton g_btnWipeH;
    static IXButton g_btnWipeV;
    static const IXButton* g_buttons[4];

    static IXSlider g_sliderDuration;
    static const IXSlider* g_sliders[1];

    static int  g_returnToMenu = 0;
    static char g_statusLine[256] = "Transition test ready.";
    static char g_helpLine[256]   = "Click a button to trigger a transition | ESC menu (instant)";

    struct TLayoutCache { int valid, x, y, w, h; };
    static TLayoutCache g_cache = {0,0,0,0,0};

    static void RefreshLayout()
    {
        int x, y, w, h;
        Renderer_GetOverlayContentRect(&x, &y, &w, &h);
        g_cache = {1, x, y, w, h};

        IXLayout layout;
        // Botoes comecam apos as linhas de texto do overlay (~96px)
        Layout_BeginVertical(&layout, x, y + 96, w, h - 96, 8);

        int bx, by, bw, bh;
        Layout_Next(&layout, 36, &bx, &by, &bw, &bh); Button_SetBounds(&g_btnFadeBlack, bx, by, bw, bh);
        Layout_Next(&layout, 36, &bx, &by, &bw, &bh); Button_SetBounds(&g_btnFadeWhite, bx, by, bw, bh);
        Layout_Next(&layout, 36, &bx, &by, &bw, &bh); Button_SetBounds(&g_btnWipeH,     bx, by, bw, bh);
        Layout_Next(&layout, 36, &bx, &by, &bw, &bh); Button_SetBounds(&g_btnWipeV,     bx, by, bw, bh);

        // Slider de duracao com margem extra acima
        Layout_Skip(&layout, 12);
        Layout_Next(&layout, 40, &bx, &by, &bw, &bh); Slider_SetBounds(&g_sliderDuration, bx, by, bw, bh);
    }

    static void EnsureLayout()
    {
        int x, y, w, h;
        Renderer_GetOverlayContentRect(&x, &y, &w, &h);
        if (!g_cache.valid || g_cache.x != x || g_cache.y != y || g_cache.w != w || g_cache.h != h)
            RefreshLayout();
    }

    static float GetDurationSeconds()
    {
        return (float)Slider_GetValue(&g_sliderDuration) / 1000.0f;
    }

    static void DoTransition(int type, unsigned int color)
    {
        if (Transition_IsActive()) return;
        float dur = GetDurationSeconds();
        Transition_Request(IX_SCENE_MENU, type, dur, color);
        std::snprintf(g_statusLine, sizeof(g_statusLine),
            "Transitioning to Menu (type=%d, dur=%.2fs)...", type, dur);
    }
}

// ---------------------------------------------------------------------------
//  API publica
// ---------------------------------------------------------------------------

void DebugTransitionTest_Initialize()
{
    Button_Init(&g_btnFadeBlack, 0, 0, 0, 0, "Fade Black  ->  Menu");
    Button_Init(&g_btnFadeWhite, 0, 0, 0, 0, "Fade White  ->  Menu");
    Button_Init(&g_btnWipeH,     0, 0, 0, 0, "Wipe Horizontal  ->  Menu");
    Button_Init(&g_btnWipeV,     0, 0, 0, 0, "Wipe Vertical  ->  Menu");

    // Azul escuro para fade preto
    Button_SetColors(&g_btnFadeBlack, 0xCC182030, 0xCC203050, 0xCC2C4470);
    // Cinza quente para fade branco
    Button_SetColors(&g_btnFadeWhite, 0xCC383838, 0xCC505050, 0xCC686868);
    // Verde musgo para wipe H
    Button_SetColors(&g_btnWipeH, 0xCC1E3020, 0xCC2A4430, 0xCC386040);
    // Roxo escuro para wipe V
    Button_SetColors(&g_btnWipeV, 0xCC2A1840, 0xCC3C2260, 0xCC522E80);

    g_buttons[0] = &g_btnFadeBlack;
    g_buttons[1] = &g_btnFadeWhite;
    g_buttons[2] = &g_btnWipeH;
    g_buttons[3] = &g_btnWipeV;

    // Slider: duracao em ms (100ms a 1500ms, padrao 400ms)
    Slider_Init(&g_sliderDuration, 0, 0, 0, 0, 100, 1500, 400, "Duration (ms)");
    g_sliders[0] = &g_sliderDuration;

    g_returnToMenu  = 0;
    g_cache.valid   = 0;
    std::snprintf(g_statusLine, sizeof(g_statusLine), "Transition test ready.");
    std::snprintf(g_helpLine,   sizeof(g_helpLine),   "Click a button to trigger a transition | ESC = instant menu");
    EnsureLayout();
}

void DebugTransitionTest_InvalidateLayout()
{
    g_cache.valid = 0;
}

void DebugTransitionTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased)
{
    EnsureLayout();

    // ESC volta direto sem transicao
    if (Input_WasKeyPressed(VK_ESCAPE))
    {
        g_returnToMenu = 1;
        return;
    }

    // Nao processa input enquanto transicao esta ativa
    if (Transition_IsActive()) return;

    Slider_Handle(&g_sliderDuration, mouseX, mouseY, mouseDown);

    if (Button_Handle(&g_btnFadeBlack, mouseX, mouseY, mouseDown, mouseReleased)) DoTransition(IX_TRANSITION_FADE,   0x000000);
    if (Button_Handle(&g_btnFadeWhite, mouseX, mouseY, mouseDown, mouseReleased)) DoTransition(IX_TRANSITION_FADE,   0xFFFFFF);
    if (Button_Handle(&g_btnWipeH,     mouseX, mouseY, mouseDown, mouseReleased)) DoTransition(IX_TRANSITION_WIPE_H, 0x000000);
    if (Button_Handle(&g_btnWipeV,     mouseX, mouseY, mouseDown, mouseReleased)) DoTransition(IX_TRANSITION_WIPE_V, 0x000000);
}

void DebugTransitionTest_ApplyRendererState(float renderFPS, float frameMs)
{
    EnsureLayout();
    Renderer_SetOverlayText("InteractiveX r0.2", "Transition test", g_statusLine);
    Renderer_SetHelpText(g_helpLine);
    Renderer_SetDisplayOptions(1, 1, 1);
    Renderer_SetControlButton(NULL);
    Renderer_SetControlButtons(g_buttons, 4);
    Renderer_SetControlSliders(g_sliders, 1);
    Renderer_SetOverlayMetrics(renderFPS, frameMs, 0.0f);
}

int  DebugTransitionTest_ShouldReturnToMenu() { return g_returnToMenu; }
void DebugTransitionTest_ClearReturnToMenu()  { g_returnToMenu = 0;    }
