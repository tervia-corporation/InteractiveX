#include "entity_test.h"
#include "../core/entity.h"
#include "../core/input.h"
#include "../core/timer.h"
#include "../renderer.h"
#include "../gui.h"
#include <cstdio>
#include <cstring>
#include <math.h>

// ---------------------------------------------------------------------------
//  Configuracao
// ---------------------------------------------------------------------------
#define ENTITY_TEST_COUNT    12
#define PLAYER_SPEED        180.0f
#define NPC_SPEED_MIN        60.0f
#define NPC_SPEED_RANGE     100.0f

#define MODE_AABB    1
#define MODE_CIRCLE  2
#define MODE_POINT   3
#define MODE_RAY     4

static const unsigned int NPC_COLORS[] = {
    0xFFE05050, 0xFF50B0E0, 0xFF50E070, 0xFFE0B050,
    0xFFB050E0, 0xFFE07050, 0xFF50E0D0, 0xFFE050B0,
    0xFF80A0E0, 0xFF80E080, 0xFFE0E050,
};

// ---------------------------------------------------------------------------
//  Estado
// ---------------------------------------------------------------------------
static IXEntity      g_entityBuf[ENTITY_TEST_COUNT];
static IXEntityList  g_entities;

static int  g_mode         = MODE_AABB;
static int  g_returnToMenu = 0;
static char g_statusLine[256] = "Entity test ready.";
static char g_helpLine[256]   = "1 AABB  2 Circle  3 Point  4 Ray | WASD move | ESC menu";

static int g_areaX = 0, g_areaY = 0, g_areaW = 0, g_areaH = 0;
static int g_mouseX = 0, g_mouseY = 0;

// ---------------------------------------------------------------------------
//  Helpers
// ---------------------------------------------------------------------------
static unsigned int g_rngState = 0xDEADBEEF;
static float RandFloat(float mn, float mx)
{
    g_rngState = g_rngState * 1664525u + 1013904223u;
    float t = (float)(g_rngState & 0xFFFF) / (float)0xFFFF;
    return mn + t * (mx - mn);
}
static float RandSign()
{
    g_rngState = g_rngState * 1664525u + 1013904223u;
    return (g_rngState & 1) ? 1.0f : -1.0f;
}

static void RefreshGameArea()
{
    int cw = 0, ch = 0;
    Renderer_GetClientSize(&cw, &ch);
    g_areaX = 376; g_areaY = 0;
    g_areaW = cw - g_areaX; g_areaH = ch;
    if (g_areaW < 64) g_areaW = 64;
    if (g_areaH < 64) g_areaH = 64;
}

static void BounceEntity(IXEntity* e)
{
    if (e->x < (float)g_areaX)           { e->x=(float)g_areaX;                   e->vx=(e->vx<0)?-e->vx:e->vx; }
    if (e->x+e->w > g_areaX+g_areaW)     { e->x=(float)(g_areaX+g_areaW-e->w);   e->vx=(e->vx>0)?-e->vx:e->vx; }
    if (e->y < (float)g_areaY)           { e->y=(float)g_areaY;                   e->vy=(e->vy<0)?-e->vy:e->vy; }
    if (e->y+e->h > g_areaY+g_areaH)     { e->y=(float)(g_areaY+g_areaH-e->h);   e->vy=(e->vy>0)?-e->vy:e->vy; }
}

static void DrawCircleOutline(float cx, float cy, float r, unsigned int col)
{
    const int STEPS = 32;
    for (int i = 0; i < STEPS; i++)
    {
        float a  = (float)i / (float)STEPS * 6.28318f;
        int   px = (int)(cx + cosf(a) * r);
        int   py = (int)(cy + sinf(a) * r);
        GUI_DrawFilledRect(px-1, py-1, 2, 2, col);
    }
}

static const char* ModeName(int m)
{
    switch (m) {
        case MODE_AABB:   return "AABB";
        case MODE_CIRCLE: return "Circle";
        case MODE_POINT:  return "Point-in-Rect";
        case MODE_RAY:    return "Ray vs AABB";
    }
    return "?";
}

// ---------------------------------------------------------------------------
//  Public API
// ---------------------------------------------------------------------------
void DebugEntityTest_Initialize()
{
    g_returnToMenu = 0;
    g_rngState     = 0xDEADBEEF;
    g_mode         = MODE_AABB;

    EntityList_Init(&g_entities, g_entityBuf, ENTITY_TEST_COUNT);
    RefreshGameArea();

    // Player (indice 0)
    {
        int idx = EntityList_Add(&g_entities);
        IXEntity* p = &g_entities.entities[idx];
        Entity_SetPosition(p, g_areaX + g_areaW*0.5f-20.0f, g_areaY + g_areaH*0.5f-20.0f);
        Entity_SetSize(p, 40, 40);
        Entity_SetTint(p, 0xFFFFFFFF);
    }

    // NPCs
    int numColors = (int)(sizeof(NPC_COLORS)/sizeof(NPC_COLORS[0]));
    for (int i = 1; i < ENTITY_TEST_COUNT; i++)
    {
        int idx = EntityList_Add(&g_entities);
        IXEntity* e = &g_entities.entities[idx];
        int sz = 24 + (int)RandFloat(0.0f, 24.0f);
        Entity_SetPosition(e, g_areaX+RandFloat(0,(float)(g_areaW-sz)), g_areaY+RandFloat(0,(float)(g_areaH-sz)));
        Entity_SetSize(e, sz, sz);
        Entity_SetVelocity(e, RandSign()*RandFloat(NPC_SPEED_MIN,NPC_SPEED_MIN+NPC_SPEED_RANGE),
                              RandSign()*RandFloat(NPC_SPEED_MIN,NPC_SPEED_MIN+NPC_SPEED_RANGE));
        Entity_SetTint(e, NPC_COLORS[(i-1)%numColors]);
    }

    std::snprintf(g_statusLine, sizeof(g_statusLine), "Mode: AABB | %d entities", EntityList_GetCount(&g_entities));
    std::snprintf(g_helpLine,   sizeof(g_helpLine),   "1 AABB  2 Circle  3 Point-in-Rect  4 Ray vs AABB | WASD move | ESC menu");
}

void DebugEntityTest_InvalidateLayout() {}

void DebugEntityTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased)
{
    (void)mouseDown; (void)mouseReleased;
    if (Input_WasKeyPressed(VK_ESCAPE)) { g_returnToMenu = 1; return; }

    if (Input_WasKeyPressed('1')) g_mode = MODE_AABB;
    if (Input_WasKeyPressed('2')) g_mode = MODE_CIRCLE;
    if (Input_WasKeyPressed('3')) g_mode = MODE_POINT;
    if (Input_WasKeyPressed('4')) g_mode = MODE_RAY;

    g_mouseX = mouseX;
    g_mouseY = mouseY;

    RefreshGameArea();
    float dt = Timer_GetDeltaSeconds();

    IXEntity* player = &g_entities.entities[0];
    player->vx = 0; player->vy = 0;
    if (Input_IsKeyDown('W')||Input_IsKeyDown(VK_UP))    player->vy = -PLAYER_SPEED;
    if (Input_IsKeyDown('S')||Input_IsKeyDown(VK_DOWN))  player->vy =  PLAYER_SPEED;
    if (Input_IsKeyDown('A')||Input_IsKeyDown(VK_LEFT))  player->vx = -PLAYER_SPEED;
    if (Input_IsKeyDown('D')||Input_IsKeyDown(VK_RIGHT)) player->vx =  PLAYER_SPEED;

    EntityList_UpdateAll(&g_entities, dt);
    for (int i = 0; i < g_entities.count; i++)
        BounceEntity(&g_entities.entities[i]);

    // Conta hits para o status
    int hits = 0;
    float pcx = player->x + player->w*0.5f;
    float pcy = player->y + player->h*0.5f;
    float pmx = (float)g_mouseX, pmy = (float)g_mouseY;
    float rdx = pmx-pcx, rdy = pmy-pcy;
    float rlen = sqrtf(rdx*rdx+rdy*rdy);
    if (rlen > 0.5f) { rdx/=rlen; rdy/=rlen; } else { rdx=1; rdy=0; }

    for (int i = 1; i < g_entities.count; i++)
    {
        IXEntity* e = &g_entities.entities[i];
        switch (g_mode) {
            case MODE_AABB:   hits += Entity_Overlaps(player, e); break;
            case MODE_CIRCLE: hits += Entity_OverlapsCircle(player, e); break;
            case MODE_POINT:  hits += Entity_ContainsPoint(e, pmx, pmy); break;
            case MODE_RAY:    hits += Entity_RayHit(e, pcx, pcy, rdx, rdy, rlen, NULL); break;
        }
    }

    std::snprintf(g_statusLine, sizeof(g_statusLine),
        "Mode: %s | Hits: %d | Player: %.0f,%.0f",
        ModeName(g_mode), hits, player->x, player->y);
}

void DebugEntityTest_Render()
{
    IXEntity* player = &g_entities.entities[0];
    float pcx = player->x + player->w*0.5f;
    float pcy = player->y + player->h*0.5f;
    float pmx = (float)g_mouseX, pmy = (float)g_mouseY;
    float rdx = pmx-pcx, rdy = pmy-pcy;
    float rlen = sqrtf(rdx*rdx+rdy*rdy);
    if (rlen > 0.5f) { rdx/=rlen; rdy/=rlen; } else { rdx=1; rdy=0; }

    // NPCs
    for (int i = 1; i < g_entities.count; i++)
    {
        IXEntity* e = &g_entities.entities[i];
        int hit = 0;
        switch (g_mode) {
            case MODE_AABB:   hit = Entity_Overlaps(player, e); break;
            case MODE_CIRCLE: hit = Entity_OverlapsCircle(player, e); break;
            case MODE_POINT:  hit = Entity_ContainsPoint(e, pmx, pmy); break;
            case MODE_RAY:    hit = Entity_RayHit(e, pcx, pcy, rdx, rdy, rlen, NULL); break;
        }

        if (hit) {
            unsigned int saved = e->tint;
            e->tint = 0xFFFFFFFF;
            Entity_Render(e);
            e->tint = saved;
        } else
            Entity_Render(e);

        if (g_mode == MODE_CIRCLE) {
            float ecx = e->x + e->w*0.5f, ecy = e->y + e->h*0.5f;
            float er  = (float)(e->w < e->h ? e->w : e->h) * 0.5f;
            DrawCircleOutline(ecx, ecy, er, hit ? 0xFFFFFFFF : 0x55FFFFFF);
        }
    }

    // Player
    Entity_Render(player);

    if (g_mode == MODE_CIRCLE) {
        float pr = (float)(player->w < player->h ? player->w : player->h) * 0.5f;
        DrawCircleOutline(pcx, pcy, pr, 0xFFFFFFFF);
    }

    // Modo Point: mira no mouse
    if (g_mode == MODE_POINT) {
        GUI_DrawFilledRect(g_mouseX-8, g_mouseY-1, 16, 2, 0xCCFFFFFF);
        GUI_DrawFilledRect(g_mouseX-1, g_mouseY-8,  2, 16, 0xCCFFFFFF);
    }

    // Modo Ray: linha pontilhada + ponto de impacto mais proximo
    if (g_mode == MODE_RAY && rlen > 2.0f) {
        const int STEPS = 80;
        for (int s = 0; s <= STEPS; s++) {
            float t = (float)s/(float)STEPS * rlen;
            GUI_DrawFilledRect((int)(pcx+rdx*t), (int)(pcy+rdy*t), 2, 2, 0x88FFDD44);
        }
        float tMin = rlen; int hasHit = 0;
        for (int i = 1; i < g_entities.count; i++) {
            float tHit = rlen;
            if (Entity_RayHit(&g_entities.entities[i], pcx, pcy, rdx, rdy, rlen, &tHit))
                if (!hasHit || tHit < tMin) { tMin = tHit; hasHit = 1; }
        }
        if (hasHit) {
            int hx = (int)(pcx+rdx*tMin), hy = (int)(pcy+rdy*tMin);
            GUI_DrawFilledRect(hx-4, hy-4, 8, 8, 0xFFFF4444);
        }
    }
}

void DebugEntityTest_ApplyRendererState(float renderFPS, float frameMs)
{
    Renderer_SetOverlayText("InteractiveX r0.2", "Entity test", g_statusLine);
    Renderer_SetHelpText(g_helpLine);
    Renderer_SetDisplayOptions(1, 1, 0);
    Renderer_SetControlButton(NULL);
    Renderer_SetControlButtons(NULL, 0);
    Renderer_SetControlSliders(NULL, 0);
    Renderer_SetOverlayMetrics(renderFPS, frameMs, 0.0f);
}

void DebugEntityTest_HandleKeyDown(unsigned int key)
{
    if (key == VK_ESCAPE) g_returnToMenu = 1;
}

int  DebugEntityTest_ShouldReturnToMenu() { return g_returnToMenu; }
void DebugEntityTest_ClearReturnToMenu()  { g_returnToMenu = 0;    }
