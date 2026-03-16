#include "math2d_test.h"
#include "../core/math2d.h"
#include "../core/input.h"
#include "../core/timer.h"
#include "../renderer.h"
#include "../gui.h"
#include <cstdio>
#include <cstring>
#include <math.h>

// ---------------------------------------------------------------------------
//  Layout — area de demo comeca depois do overlay (x=384)
// ---------------------------------------------------------------------------
#define DEMO_X   392
#define DEMO_Y    20
#define DEMO_W   576   // cw - DEMO_X - margem
#define DEMO_H   500

// ---------------------------------------------------------------------------
//  Estado
// ---------------------------------------------------------------------------
static int   g_returnToMenu = 0;
static char  g_statusLine[256] = "";
static char  g_helpLine[128]   = "Mouse: mover ponto | ESC: menu";

// Ponto de referencia fixo (centro da area de demo)
static IXVec2 g_origin;

// Ponto controlado pelo mouse
static IXVec2 g_mouse;

// Angulo de rotacao animado
static float g_rotAngle = 0.0f;

// Lerp animado (0..1 vai-volta)
static float g_lerpT    = 0.0f;
static float g_lerpDir  = 1.0f;

// Retangulo de teste de overlap
static IXVec2 g_rectPos;
static float  g_rectW = 80.0f, g_rectH = 50.0f;

// Circulo de teste de overlap
static IXVec2 g_circlePos;
static float  g_circleR = 40.0f;

// ---------------------------------------------------------------------------
//  Helpers visuais simples
// ---------------------------------------------------------------------------
static void DrawLine(float x0, float y0, float x1, float y1,
                     unsigned int color, int thickness)
{
    float dx = x1 - x0, dy = y1 - y0;
    float len = sqrtf(dx*dx + dy*dy);
    if (len < 1.0f) return;
    // Desenha como serie de pontos ao longo da linha
    int steps = (int)len + 1;
    for (int i = 0; i <= steps; i++)
    {
        float t = (float)i / (float)steps;
        int px = (int)(x0 + dx * t) - thickness/2;
        int py = (int)(y0 + dy * t) - thickness/2;
        GUI_DrawFilledRect(px, py, thickness, thickness, color);
    }
}

static void DrawCircleOutline(float cx, float cy, float r, unsigned int color)
{
    int steps = (int)(r * 2.0f * IX_PI / 4.0f) + 8;
    if (steps < 8) steps = 8;
    if (steps > 128) steps = 128;
    for (int i = 0; i < steps; i++)
    {
        float a0 = (float)i       / (float)steps * IX_TAU;
        float a1 = (float)(i + 1) / (float)steps * IX_TAU;
        DrawLine(cx + cosf(a0)*r, cy + sinf(a0)*r,
                 cx + cosf(a1)*r, cy + sinf(a1)*r, color, 1);
    }
}

static void DrawArrow(IXVec2 from, IXVec2 to, unsigned int color)
{
    DrawLine(from.x, from.y, to.x, to.y, color, 1);
    // Cabeca da seta
    IXVec2 dir = Vec2_Normalize(Vec2_Sub(to, from));
    IXVec2 left  = Vec2_Scale(Vec2_Rotate(dir, IX_DEG2RAD( 145.0f)), 8.0f);
    IXVec2 right = Vec2_Scale(Vec2_Rotate(dir, IX_DEG2RAD(-145.0f)), 8.0f);
    DrawLine(to.x, to.y, to.x + left.x,  to.y + left.y,  color, 1);
    DrawLine(to.x, to.y, to.x + right.x, to.y + right.y, color, 1);
}

static void DrawDot(float x, float y, int r, unsigned int color)
{
    GUI_DrawFilledRect((int)x - r, (int)y - r, r*2, r*2, color);
}

static void DrawLabel(float x, float y, const char* text, unsigned int color)
{
    GUI_DrawTextLine((int)x + 4, (int)y - 14, text, color);
}

// ---------------------------------------------------------------------------
//  Public API
// ---------------------------------------------------------------------------
void DebugMath2DTest_Initialize()
{
    g_returnToMenu = 0;
    g_rotAngle = 0.0f;
    g_lerpT = 0.0f; g_lerpDir = 1.0f;

    int cw = 0, ch = 0;
    Renderer_GetClientSize(&cw, &ch);

    // Origem no centro da area de demo
    g_origin.x = DEMO_X + (cw - DEMO_X) * 0.5f;
    g_origin.y = DEMO_Y + (float)(ch - DEMO_Y) * 0.5f;

    g_mouse = Vec2(g_origin.x + 100.0f, g_origin.y - 60.0f);

    // Retangulo fixo
    g_rectPos.x = g_origin.x - 160.0f;
    g_rectPos.y = g_origin.y +  40.0f;

    // Circulo fixo
    g_circlePos.x = g_origin.x + 120.0f;
    g_circlePos.y = g_origin.y +  80.0f;
}

void DebugMath2DTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased)
{
    (void)mouseDown; (void)mouseReleased;
    if (Input_WasKeyPressed(VK_ESCAPE)) { g_returnToMenu = 1; return; }

    float dt = Timer_GetDeltaSeconds();

    // Mouse controla o ponto de teste
    g_mouse.x = (float)mouseX;
    g_mouse.y = (float)mouseY;

    // Animacoes
    g_rotAngle += dt * 1.2f;
    if (g_rotAngle > IX_TAU) g_rotAngle -= IX_TAU;

    g_lerpT += g_lerpDir * dt * 0.7f;
    if (g_lerpT >= 1.0f) { g_lerpT = 1.0f; g_lerpDir = -1.0f; }
    if (g_lerpT <= 0.0f) { g_lerpT = 0.0f; g_lerpDir =  1.0f; }

    // Calcula info para status
    IXVec2 delta = Vec2_Sub(g_mouse, g_origin);
    float dist   = Vec2_Length(delta);
    float angleDeg = IX_RAD2DEG(Vec2_Angle(delta));

    int overRect = IX_OverlapCircle(g_mouse.x, g_mouse.y, 8.0f,
                                     g_rectPos.x + g_rectW*0.5f,
                                     g_rectPos.y + g_rectH*0.5f,
                                     IX_Max(g_rectW, g_rectH)*0.5f);
    (void)overRect;

    int overCircle = IX_PointInCircle(g_mouse.x, g_mouse.y,
                                       g_circlePos.x, g_circlePos.y, g_circleR);

    std::snprintf(g_statusLine, sizeof(g_statusLine),
        "dist=%.1f | angulo=%.1f graus | mouse %s",
        dist, angleDeg,
        overCircle ? "DENTRO do circulo" : "fora do circulo");
}

void DebugMath2DTest_Render()
{
    int cw = 0, ch = 0;
    Renderer_GetClientSize(&cw, &ch);

    // --- grade de fundo leve ---
    for (int x = DEMO_X; x < cw; x += 40)
        GUI_DrawFilledRect(x, DEMO_Y, 1, ch - DEMO_Y, 0x0AFFFFFF);
    for (int y = DEMO_Y; y < ch; y += 40)
        GUI_DrawFilledRect(DEMO_X, y, cw - DEMO_X, 1, 0x0AFFFFFF);

    // ===================================================
    //  1. VETOR: origem -> mouse
    // ===================================================
    IXVec2 delta = Vec2_Sub(g_mouse, g_origin);
    float  dist  = Vec2_Length(delta);
    IXVec2 norm  = Vec2_Normalize(delta);

    DrawArrow(g_origin, g_mouse, 0xFF88BBFF);
    DrawDot(g_origin.x, g_origin.y, 5, 0xFFFFFFFF);
    DrawLabel(g_origin.x, g_origin.y, "origem", 0xFF888888);
    DrawDot(g_mouse.x, g_mouse.y, 5, 0xFF88BBFF);

    // Label distancia no meio do vetor
    {
        IXVec2 mid = Vec2_Lerp(g_origin, g_mouse, 0.5f);
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.0fpx", dist);
        GUI_DrawTextLine((int)mid.x + 4, (int)mid.y - 4, buf, 0xFF88BBFF);
    }

    // ===================================================
    //  2. ROTACAO: 4 pontos orbitando a origem
    // ===================================================
    float orbitR = 60.0f;
    unsigned int orbitColors[] = { 0xFFFF6644, 0xFF44FF88, 0xFF4488FF, 0xFFFFDD44 };
    for (int i = 0; i < 4; i++)
    {
        float a = g_rotAngle + IX_DEG2RAD(i * 90.0f);
        IXVec2 p = Vec2_Add(g_origin, Vec2_FromAngle(a, orbitR));
        DrawLine(g_origin.x, g_origin.y, p.x, p.y, orbitColors[i] & 0x44FFFFFF, 1);
        DrawDot(p.x, p.y, 4, orbitColors[i]);
    }
    DrawLabel(g_origin.x + orbitR + 4, g_origin.y - orbitR - 4,
              "Vec2_Rotate", 0xFF888888);

    // ===================================================
    //  3. LERP animado: ponto A -> ponto B
    // ===================================================
    IXVec2 lerpA = Vec2(g_origin.x - 120.0f, g_origin.y - 100.0f);
    IXVec2 lerpB = Vec2(g_origin.x + 120.0f, g_origin.y - 100.0f);
    IXVec2 lerpP = Vec2_Lerp(lerpA, lerpB, g_lerpT);
    IXVec2 smthP = Vec2_Lerp(lerpA, lerpB, IX_SmoothStep(g_lerpT));

    DrawLine(lerpA.x, lerpA.y, lerpB.x, lerpB.y, 0x33FFFFFF, 1);
    DrawDot(lerpA.x, lerpA.y, 4, 0xFFAAAAAA);
    DrawDot(lerpB.x, lerpB.y, 4, 0xFFAAAAAA);
    DrawDot(lerpP.x, lerpP.y, 6, 0xFFFF8844);  // lerp linear
    DrawDot(smthP.x, smthP.y, 6, 0xFF88FF44);  // smooth step
    DrawLabel(lerpA.x,  lerpA.y, "A", 0xFFAAAAAA);
    DrawLabel(lerpB.x,  lerpB.y, "B", 0xFFAAAAAA);
    DrawLabel(lerpP.x,  lerpP.y - 4, "lerp", 0xFFFF8844);
    DrawLabel(smthP.x,  smthP.y + 14, "smooth", 0xFF88FF44);

    // ===================================================
    //  4. REFLEXAO: vetor mouse refletido numa normal vertical
    // ===================================================
    IXVec2 reflNormal = Vec2_Right();
    IXVec2 reflInput  = Vec2_Normalize(Vec2_Sub(g_mouse, g_origin));
    IXVec2 reflResult = Vec2_Reflect(Vec2_Neg(reflInput), reflNormal);
    IXVec2 reflBase   = Vec2(g_origin.x + 150.0f, g_origin.y + 10.0f);

    // Normal
    DrawLine(reflBase.x, reflBase.y - 40.0f,
             reflBase.x, reflBase.y + 40.0f, 0x88FFFFFF, 1);
    DrawLabel(reflBase.x, reflBase.y - 44, "normal", 0xFF888888);

    DrawArrow(reflBase,
              Vec2_Add(reflBase, Vec2_Scale(reflInput, 55.0f)), 0xFFFF4444);
    DrawArrow(reflBase,
              Vec2_Add(reflBase, Vec2_Scale(reflResult, 55.0f)), 0xFF44FFAA);
    DrawLabel(reflBase.x + 8, reflBase.y + 40, "Vec2_Reflect", 0xFF888888);

    // ===================================================
    //  5. OVERLAP: AABB + Circulo vs mouse
    // ===================================================
    // AABB
    int aabbHit = IX_OverlapCircle(g_mouse.x, g_mouse.y, 6.0f,
                                    g_rectPos.x + g_rectW*0.5f,
                                    g_rectPos.y + g_rectH*0.5f,
                                    IX_Max(g_rectW, g_rectH)*0.6f)
                  || IX_PointInAABB(g_mouse.x, g_mouse.y,
                                    g_rectPos.x, g_rectPos.y, g_rectW, g_rectH);

    unsigned int rectColor = aabbHit ? 0x88FF8844 : 0x3344AAFF;
    unsigned int rectBorder = aabbHit ? 0xFFFF8844 : 0xFF4488FF;
    GUI_DrawFilledRect((int)g_rectPos.x, (int)g_rectPos.y,
                       (int)g_rectW, (int)g_rectH, rectColor);
    GUI_DrawOutlinedRect((int)g_rectPos.x, (int)g_rectPos.y,
                         (int)g_rectW, (int)g_rectH, rectBorder);
    DrawLabel(g_rectPos.x, g_rectPos.y, "AABB", rectBorder);

    // Circulo
    int circHit = IX_PointInCircle(g_mouse.x, g_mouse.y,
                                    g_circlePos.x, g_circlePos.y, g_circleR);
    unsigned int circColor  = circHit ? 0x88FFDD44 : 0x3388FF44;
    unsigned int circBorder = circHit ? 0xFFFFDD44 : 0xFF44FF88;
    GUI_DrawFilledRect((int)(g_circlePos.x - g_circleR), (int)(g_circlePos.y - g_circleR),
                       (int)(g_circleR*2), (int)(g_circleR*2), circColor & 0x44FFFFFF);
    DrawCircleOutline(g_circlePos.x, g_circlePos.y, g_circleR, circBorder);
    DrawLabel(g_circlePos.x, g_circlePos.y - g_circleR - 4, "Circulo", circBorder);

    // ===================================================
    //  6. RAY CAST: ray da origem ate o mouse, testa AABB
    // ===================================================
    float tHit = 0.0f;
    float rayLen = dist + 1.0f;
    int rayHit = 0;
    if (rayLen > 1.0f)
        rayHit = IX_RayAABB(g_origin.x, g_origin.y,
                             norm.x, norm.y, rayLen,
                             g_rectPos.x, g_rectPos.y, g_rectW, g_rectH, &tHit);

    if (rayHit)
    {
        IXVec2 hitPt = Vec2_Add(g_origin, Vec2_Scale(norm, tHit));
        DrawDot(hitPt.x, hitPt.y, 6, 0xFFFF4488);
        DrawLabel(hitPt.x, hitPt.y, "hit!", 0xFFFF4488);
    }

    // ===================================================
    //  Painel info canto superior direito da area
    // ===================================================
    {
        int ix = cw - 220, iy = DEMO_Y + 8;
        GUI_DrawFilledRect(ix - 4, iy - 4, 218, 112, 0x22000000);
        char buf[64];
        float angleDeg = IX_RAD2DEG(Vec2_Angle(delta));
        std::snprintf(buf, sizeof(buf), "dist:   %.1f", dist);
        GUI_DrawTextLine(ix, iy,      buf, 0xFFCCCCCC); iy += 18;
        std::snprintf(buf, sizeof(buf), "angulo: %.1f deg", angleDeg);
        GUI_DrawTextLine(ix, iy,      buf, 0xFFCCCCCC); iy += 18;
        std::snprintf(buf, sizeof(buf), "norm:   %.2f, %.2f", norm.x, norm.y);
        GUI_DrawTextLine(ix, iy,      buf, 0xFFCCCCCC); iy += 18;
        std::snprintf(buf, sizeof(buf), "lerp t: %.2f", g_lerpT);
        GUI_DrawTextLine(ix, iy,      buf, 0xFFCCCCCC); iy += 18;
        std::snprintf(buf, sizeof(buf), "rot:    %.1f deg", IX_RAD2DEG(g_rotAngle));
        GUI_DrawTextLine(ix, iy,      buf, 0xFFCCCCCC); iy += 18;
        std::snprintf(buf, sizeof(buf), "rayHit: %s", rayHit ? "SIM" : "nao");
        GUI_DrawTextLine(ix, iy,      buf, rayHit ? 0xFFFF4488 : 0xFF888888);
    }
}

void DebugMath2DTest_ApplyRendererState(float renderFPS, float frameMs)
{
    Renderer_SetOverlayText("InteractiveX r0.2", "Math2D test", g_statusLine);
    Renderer_SetHelpText(g_helpLine);
    Renderer_SetDisplayOptions(1, 1, 0);
    Renderer_SetControlButton(NULL);
    Renderer_SetControlButtons(NULL, 0);
    Renderer_SetControlSliders(NULL, 0);
    Renderer_SetOverlayMetrics(renderFPS, frameMs, 0.0f);
}

int  DebugMath2DTest_ShouldReturnToMenu() { return g_returnToMenu; }
void DebugMath2DTest_ClearReturnToMenu()  { g_returnToMenu = 0; }
