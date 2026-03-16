#include "transition.h"
#include "scene.h"
#include "math2d.h"
#include "../gui.h"

// ---------------------------------------------------------------------------
//  Estado interno
// ---------------------------------------------------------------------------
static int          g_active    = 0;
static int          g_type      = IX_TRANSITION_FADE;
static int          g_toScene   = 0;
static float        g_duration  = 0.4f;
static float        g_elapsed   = 0.0f;
static unsigned int g_color     = 0x000000;  // RGB, sem alpha
static int          g_switched  = 0;         // troca de cena ja foi feita?

// ---------------------------------------------------------------------------
//  Helpers
// ---------------------------------------------------------------------------

// Retorna o progresso suavizado da fase atual (0..1).
// progress: posicao normalizada dentro da duracao total (0..1).
// Fase 1 (0..0.5): cobrindo a tela   -> resultado 0..1
// Fase 2 (0.5..1): descobrindo a tela -> resultado 1..0
static float PhaseAlpha(float progress)
{
    if (progress < 0.5f)
        return IX_SmoothStep(progress * 2.0f);
    else
        return IX_SmoothStep((1.0f - progress) * 2.0f);
}

// ---------------------------------------------------------------------------
//  API publica
// ---------------------------------------------------------------------------

void Transition_Request(int toScene, int type, float durationSeconds, unsigned int color)
{
    if (g_active) return;
    g_active   = 1;
    g_type     = type;
    g_toScene  = toScene;
    g_duration = durationSeconds > 0.05f ? durationSeconds : 0.05f;
    g_elapsed  = 0.0f;
    g_color    = color & 0x00FFFFFF;
    g_switched = 0;
}

void Transition_FadeBlack(int toScene, float durationSeconds)
{
    Transition_Request(toScene, IX_TRANSITION_FADE, durationSeconds, 0x000000);
}

void Transition_FadeWhite(int toScene, float durationSeconds)
{
    Transition_Request(toScene, IX_TRANSITION_FADE, durationSeconds, 0xFFFFFF);
}

int Transition_IsActive()
{
    return g_active;
}

// ---------------------------------------------------------------------------
//  Update -- chamado pelo engine.cpp a cada frame
// ---------------------------------------------------------------------------

void Transition_Update(float deltaSeconds)
{
    if (!g_active) return;

    g_elapsed += deltaSeconds;
    float progress = IX_Clamp01(g_elapsed / g_duration);

    // Troca a cena no ponto medio (tela completamente coberta)
    if (!g_switched && progress >= 0.5f)
    {
        Scene_SetCurrent(g_toScene);
        Scene_OnEnterCurrent();
        g_switched = 1;
    }

    if (g_elapsed >= g_duration)
    {
        g_active  = 0;
        g_elapsed = 0.0f;
    }
}

// ---------------------------------------------------------------------------
//  DrawOverlay -- chamado pelo renderer.cpp dentro de EndFrame
// ---------------------------------------------------------------------------

void Transition_DrawOverlay(int screenWidth, int screenHeight)
{
    if (!g_active) return;

    float progress = IX_Clamp01(g_elapsed / g_duration);
    float phase    = PhaseAlpha(progress);

    if (g_type == IX_TRANSITION_FADE)
    {
        // Overlay solido com alpha variavel
        unsigned int a    = (unsigned int)(phase * 255.0f);
        if (a > 255) a = 255;
        unsigned int argb = (a << 24) | g_color;
        GUI_DrawFilledRect(0, 0, screenWidth, screenHeight, argb);
    }
    else if (g_type == IX_TRANSITION_WIPE_H)
    {
        // Fase 1: barra cresce da esquerda para direita
        // Fase 2: barra encolhe da direita para esquerda
        unsigned int argb = 0xFF000000 | g_color;
        int fillW = (int)(phase * (float)screenWidth);
        if (fillW > 0)
        {
            if (progress < 0.5f)
                GUI_DrawFilledRect(0, 0, fillW, screenHeight, argb);
            else
                GUI_DrawFilledRect(screenWidth - fillW, 0, fillW, screenHeight, argb);
        }
    }
    else if (g_type == IX_TRANSITION_WIPE_V)
    {
        // Fase 1: barra cresce de cima para baixo
        // Fase 2: barra encolhe de baixo para cima
        unsigned int argb = 0xFF000000 | g_color;
        int fillH = (int)(phase * (float)screenHeight);
        if (fillH > 0)
        {
            if (progress < 0.5f)
                GUI_DrawFilledRect(0, 0, screenWidth, fillH, argb);
            else
                GUI_DrawFilledRect(0, screenHeight - fillH, screenWidth, fillH, argb);
        }
    }
}
