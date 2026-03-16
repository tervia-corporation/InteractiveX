#include "particles_test.h"
#include "../core/particles.h"
#include "../core/input.h"
#include "../core/timer.h"
#include "../renderer.h"
#include "../gui.h"
#include <cstdio>
#include <cstring>

// ---------------------------------------------------------------------------
//  Configuracao
// ---------------------------------------------------------------------------
#define POOL_FIRE    300
#define POOL_SMOKE   150
#define POOL_SPARKS  200
#define POOL_BURST   400

// ---------------------------------------------------------------------------
//  Buffers e emissores
// ---------------------------------------------------------------------------
static IXParticle g_fireBuf  [POOL_FIRE];
static IXParticle g_smokeBuf [POOL_SMOKE];
static IXParticle g_sparksBuf[POOL_SPARKS];
static IXParticle g_burstBuf [POOL_BURST];

static IXEmitter  g_fire;
static IXEmitter  g_smoke;
static IXEmitter  g_sparks;
static IXEmitter  g_burst;

static int  g_returnToMenu = 0;
static char g_statusLine[256] = "Particles test ready.";
static char g_helpLine[256]   = "Click: burst | 1-4 toggle emissores | R reset | ESC menu";

// Posicoes dos emissores (centro da area de jogo)
static float g_centerX = 680.0f;
static float g_centerY = 400.0f;

// ---------------------------------------------------------------------------
//  Inicializacao dos emissores
// ---------------------------------------------------------------------------
static void SetupFire(float cx, float cy)
{
    Emitter_Init(&g_fire, g_fireBuf, POOL_FIRE);
    Emitter_SetPosition(&g_fire, cx, cy);
    Emitter_SetSpread(&g_fire, 30.0f, 4.0f);
    Emitter_SetSpawnRate(&g_fire, 80.0f);
    Emitter_SetLifetime(&g_fire, 0.4f, 1.0f);
    Emitter_SetVelocity(&g_fire, -15.0f, 15.0f, -160.0f, -60.0f);
    Emitter_SetGravity(&g_fire, 0.0f, -30.0f);   // leve sustentacao
    Emitter_SetSize(&g_fire, 10, 2);
    Emitter_SetColorOverLife(&g_fire, 0xFFFF9900, 0x00FF2200);
}

static void SetupSmoke(float cx, float cy)
{
    Emitter_Init(&g_smoke, g_smokeBuf, POOL_SMOKE);
    Emitter_SetPosition(&g_smoke, cx, cy - 40.0f);
    Emitter_SetSpread(&g_smoke, 20.0f, 4.0f);
    Emitter_SetSpawnRate(&g_smoke, 20.0f);
    Emitter_SetLifetime(&g_smoke, 1.0f, 2.5f);
    Emitter_SetVelocity(&g_smoke, -18.0f, 18.0f, -60.0f, -20.0f);
    Emitter_SetGravity(&g_smoke, 0.0f, -10.0f);
    Emitter_SetSize(&g_smoke, 8, 24);
    Emitter_SetColorOverLife(&g_smoke, 0x88888888, 0x00444444);
}

static void SetupSparks(float cx, float cy)
{
    Emitter_Init(&g_sparks, g_sparksBuf, POOL_SPARKS);
    Emitter_SetPosition(&g_sparks, cx, cy);
    Emitter_SetSpread(&g_sparks, 8.0f, 4.0f);
    Emitter_SetSpawnRate(&g_sparks, 40.0f);
    Emitter_SetLifetime(&g_sparks, 0.3f, 0.8f);
    Emitter_SetVelocity(&g_sparks, -120.0f, 120.0f, -200.0f, 20.0f);
    Emitter_SetGravity(&g_sparks, 0.0f, 400.0f);   // queda rapida
    Emitter_SetSize(&g_sparks, 3, 1);
    Emitter_SetColorOverLife(&g_sparks, 0xFFFFEE44, 0x00FF6600);
}

static void SetupBurst(float cx, float cy)
{
    Emitter_Init(&g_burst, g_burstBuf, POOL_BURST);
    Emitter_SetPosition(&g_burst, cx, cy);
    Emitter_SetSpawnRate(&g_burst, 0.0f);  // so burst manual
    Emitter_SetLifetime(&g_burst, 0.3f, 0.9f);
    Emitter_SetVelocity(&g_burst, -200.0f, 200.0f, -200.0f, 200.0f);
    Emitter_SetGravity(&g_burst, 0.0f, 300.0f);
    Emitter_SetSize(&g_burst, 6, 1);
    Emitter_SetColorOverLife(&g_burst, 0xFFFFFFFF, 0x002080FF);
}

// ---------------------------------------------------------------------------
//  Public API
// ---------------------------------------------------------------------------
void DebugParticlesTest_Initialize()
{
    g_returnToMenu = 0;

    int cw = 0, ch = 0;
    Renderer_GetClientSize(&cw, &ch);
    g_centerX = 380.0f + (float)(cw - 380) * 0.5f;
    g_centerY = (float)ch * 0.65f;

    SetupFire(g_centerX, g_centerY);
    SetupSmoke(g_centerX, g_centerY);
    SetupSparks(g_centerX, g_centerY);
    SetupBurst(g_centerX, g_centerY);
}

void DebugParticlesTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased)
{
    (void)mouseDown;

    if (Input_WasKeyPressed(VK_ESCAPE)) { g_returnToMenu = 1; return; }

    // Toggle emissores com 1-4
    if (Input_WasKeyPressed('1')) { g_fire.active   = !g_fire.active;   if (!g_fire.active)   Emitter_Stop(&g_fire); else g_fire.active = 1; }
    if (Input_WasKeyPressed('2')) { g_smoke.active  = !g_smoke.active;  if (!g_smoke.active)  Emitter_Stop(&g_smoke); else g_smoke.active = 1; }
    if (Input_WasKeyPressed('3')) { g_sparks.active = !g_sparks.active; if (!g_sparks.active) Emitter_Stop(&g_sparks); else g_sparks.active = 1; }
    if (Input_WasKeyPressed('R')) DebugParticlesTest_Initialize();

    // Clique esquerdo: burst na posicao do mouse
    if (mouseReleased)
    {
        Emitter_SetPosition(&g_burst, (float)mouseX, (float)mouseY);
        Emitter_Burst(&g_burst, 80);
    }

    float dt = Timer_GetDeltaSeconds();
    Emitter_Update(&g_fire,   dt);
    Emitter_Update(&g_smoke,  dt);
    Emitter_Update(&g_sparks, dt);
    Emitter_Update(&g_burst,  dt);

    int total = Emitter_GetActiveCount(&g_fire)
              + Emitter_GetActiveCount(&g_smoke)
              + Emitter_GetActiveCount(&g_sparks)
              + Emitter_GetActiveCount(&g_burst);

    std::snprintf(g_statusLine, sizeof(g_statusLine),
        "Particulas ativas: %d | Fire: %d | Smoke: %d | Sparks: %d | Burst: %d",
        total,
        Emitter_GetActiveCount(&g_fire),
        Emitter_GetActiveCount(&g_smoke),
        Emitter_GetActiveCount(&g_sparks),
        Emitter_GetActiveCount(&g_burst));
}

void DebugParticlesTest_Render()
{
    int cw = 0, ch = 0;
    Renderer_GetClientSize(&cw, &ch);

    // Linha de chao visual
    GUI_DrawFilledRect(380, (int)g_centerY + 4, cw - 380, 1, 0x33FFFFFF);

    // Labels dos emissores
    GUI_DrawTextLine((int)g_centerX - 20, (int)g_centerY + 14,
        g_fire.active ? "fire [1]" : "[1] off", 0xFF888888);

    // Renderiza na ordem: smoke (atras) -> fire -> sparks -> burst (na frente)
    Emitter_Render(&g_smoke);
    Emitter_Render(&g_fire);
    Emitter_Render(&g_sparks);
    Emitter_Render(&g_burst);

    // Legenda
    int lx = 390, ly = ch - 90;
    GUI_DrawFilledRect(lx - 4, ly - 4, 280, 70, 0x22000000);
    GUI_DrawFilledRect(lx, ly + 2,  12, 12, 0xFFFF9900); GUI_DrawTextLine(lx + 18, ly,      "fire (1)",     g_fire.active   ? 0xFFFFCC88 : 0xFF555555);
    GUI_DrawFilledRect(lx, ly + 20, 12, 12, 0x88888888); GUI_DrawTextLine(lx + 18, ly + 18, "smoke (2)",    g_smoke.active  ? 0xFFCCCCCC : 0xFF555555);
    GUI_DrawFilledRect(lx, ly + 38, 12, 12, 0xFFFFEE44); GUI_DrawTextLine(lx + 18, ly + 36, "sparks (3)",   g_sparks.active ? 0xFFFFEE88 : 0xFF555555);
    GUI_DrawFilledRect(lx + 160, ly + 2,  12, 12, 0xFF2080FF); GUI_DrawTextLine(lx + 178, ly,      "burst (click)", 0xFF88BBFF);
}

void DebugParticlesTest_ApplyRendererState(float renderFPS, float frameMs)
{
    Renderer_SetOverlayText("InteractiveX r0.2", "Particles test", g_statusLine);
    Renderer_SetHelpText(g_helpLine);
    Renderer_SetDisplayOptions(1, 1, 0);
    Renderer_SetControlButton(NULL);
    Renderer_SetControlButtons(NULL, 0);
    Renderer_SetControlSliders(NULL, 0);
    Renderer_SetOverlayMetrics(renderFPS, frameMs, 0.0f);
}

void DebugParticlesTest_HandleKeyDown(unsigned int key)
{
    if (key == VK_ESCAPE) g_returnToMenu = 1;
}

int  DebugParticlesTest_ShouldReturnToMenu() { return g_returnToMenu; }
void DebugParticlesTest_ClearReturnToMenu()  { g_returnToMenu = 0; }
