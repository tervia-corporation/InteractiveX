#include "camera_test.h"
#include "../core/camera.h"
#include "../core/entity.h"
#include "../core/input.h"
#include "../core/timer.h"
#include "../renderer.h"
#include "../gui.h"
#include <cstdio>
#include <cstring>

// ---------------------------------------------------------------------------
//  Mundo
// ---------------------------------------------------------------------------
#define WORLD_W     2400
#define WORLD_H     1800
#define ENTITY_COUNT  40
#define PLAYER_SPEED 200.0f

// Cores dos objetos de mundo
static const unsigned int WORLD_COLORS[] = {
    0xFFE05050, 0xFF50B0E0, 0xFF50E070, 0xFFE0B050, 0xFFB050E0,
    0xFFE07050, 0xFF50E0D0, 0xFFE050B0, 0xFF80A0E0, 0xFF80E080,
};

// ---------------------------------------------------------------------------
//  Estado
// ---------------------------------------------------------------------------
static IXCamera     g_cam;
static IXEntity     g_entityBuf[ENTITY_COUNT];
static IXEntityList g_entities;

static int  g_returnToMenu    = 0;
static char g_statusLine[256] = "Camera test ready.";
static char g_helpLine[256]   = "";
static float g_zoomTarget     = 1.0f;

// ---------------------------------------------------------------------------
//  LCG para posicoes deterministicas
// ---------------------------------------------------------------------------
static unsigned int g_rng = 0xBEEFCAFE;
static float Rand(float lo, float hi)
{
    g_rng = g_rng * 1664525u + 1013904223u;
    float t = (float)(g_rng & 0xFFFF) / (float)0xFFFF;
    return lo + t * (hi - lo);
}
static float RandSign()
{
    g_rng = g_rng * 1664525u + 1013904223u;
    return (g_rng & 1) ? 1.0f : -1.0f;
}

// ---------------------------------------------------------------------------
//  Desenha grade do mundo (coordenadas de mundo -> tela via camera)
// ---------------------------------------------------------------------------
static void DrawWorldGrid()
{
    const int STEP = 200;
    const unsigned int LINE_COLOR = 0x22FFFFFF;
    const unsigned int BORDER_COLOR = 0x88FF8844;

    // Linhas verticais
    for (int wx = 0; wx <= WORLD_W; wx += STEP)
    {
        int sx0, sy0, sx1, sy1;
        Camera_WorldToScreen(&g_cam, (float)wx, 0.0f,         &sx0, &sy0);
        Camera_WorldToScreen(&g_cam, (float)wx, (float)WORLD_H, &sx1, &sy1);
        GUI_DrawFilledRect(sx0, sy0, 1, sy1 - sy0, LINE_COLOR);
    }
    // Linhas horizontais
    for (int wy = 0; wy <= WORLD_H; wy += STEP)
    {
        int sx0, sy0, sx1, sy1;
        Camera_WorldToScreen(&g_cam, 0.0f,         (float)wy, &sx0, &sy0);
        Camera_WorldToScreen(&g_cam, (float)WORLD_W, (float)wy, &sx1, &sy1);
        GUI_DrawFilledRect(sx0, sy0, sx1 - sx0, 1, LINE_COLOR);
    }

    // Borda do mundo
    int bx, by, bx2, by2;
    Camera_WorldToScreen(&g_cam, 0.0f, 0.0f, &bx, &by);
    Camera_WorldToScreen(&g_cam, (float)WORLD_W, (float)WORLD_H, &bx2, &by2);
    GUI_DrawOutlinedRect(bx, by, bx2 - bx, by2 - by, BORDER_COLOR);
}

// ---------------------------------------------------------------------------
//  Public API
// ---------------------------------------------------------------------------
void DebugCameraTest_Initialize()
{
    g_returnToMenu = 0;
    g_rng          = 0xBEEFCAFE;
    g_zoomTarget   = 1.0f;

    // Camera
    Camera_Init(&g_cam);
    Camera_SetWorldBounds(&g_cam, 0.0f, 0.0f, (float)WORLD_W, (float)WORLD_H);
    Camera_SetFollowLerp(&g_cam, 0.08f);

    // Entidades
    EntityList_Init(&g_entities, g_entityBuf, ENTITY_COUNT);

    // Player (indice 0) — centro do mundo
    {
        int idx = EntityList_Add(&g_entities);
        IXEntity* p = &g_entities.entities[idx];
        Entity_SetPosition(p, WORLD_W * 0.5f - 20.0f, WORLD_H * 0.5f - 20.0f);
        Entity_SetSize(p, 40, 40);
        Entity_SetTint(p, 0xFFFFFFFF);
    }

    // NPCs espalhados pelo mundo
    int numColors = (int)(sizeof(WORLD_COLORS) / sizeof(WORLD_COLORS[0]));
    for (int i = 1; i < ENTITY_COUNT; i++)
    {
        int idx = EntityList_Add(&g_entities);
        IXEntity* e = &g_entities.entities[idx];
        float px = Rand(32.0f, (float)(WORLD_W - 64));
        float py = Rand(32.0f, (float)(WORLD_H - 64));
        float vx = RandSign() * Rand(40.0f, 120.0f);
        float vy = RandSign() * Rand(40.0f, 120.0f);
        int   sz = 20 + (int)Rand(0.0f, 30.0f);
        Entity_SetPosition(e, px, py);
        Entity_SetSize(e, sz, sz);
        Entity_SetVelocity(e, vx, vy);
        Entity_SetTint(e, WORLD_COLORS[(i - 1) % numColors]);
    }

    Camera_CenterOn(&g_cam, g_entities.entities[0].x, g_entities.entities[0].y);
    Camera_Update(&g_cam, 0.0f);

    std::snprintf(g_helpLine, sizeof(g_helpLine),
        "WASD mover | Q/E zoom | F shake | ESC menu");
}

void DebugCameraTest_InvalidateLayout() {}

void DebugCameraTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased)
{
    (void)mouseDown; (void)mouseReleased;

    if (Input_WasKeyPressed(VK_ESCAPE)) { g_returnToMenu = 1; return; }

    float dt = Timer_GetDeltaSeconds();

    // --- Input player ---
    IXEntity* player = &g_entities.entities[0];
    player->vx = 0.0f;
    player->vy = 0.0f;
    if (Input_IsKeyDown('W') || Input_IsKeyDown(VK_UP))    player->vy = -PLAYER_SPEED;
    if (Input_IsKeyDown('S') || Input_IsKeyDown(VK_DOWN))  player->vy =  PLAYER_SPEED;
    if (Input_IsKeyDown('A') || Input_IsKeyDown(VK_LEFT))  player->vx = -PLAYER_SPEED;
    if (Input_IsKeyDown('D') || Input_IsKeyDown(VK_RIGHT)) player->vx =  PLAYER_SPEED;

    // Clamp player nos limites do mundo
    if (player->x < 0.0f)              player->x = 0.0f;
    if (player->y < 0.0f)              player->y = 0.0f;
    if (player->x + player->w > WORLD_W) player->x = (float)(WORLD_W - player->w);
    if (player->y + player->h > WORLD_H) player->y = (float)(WORLD_H - player->h);

    // --- Zoom ---
    if (Input_IsKeyDown('Q')) g_zoomTarget -= 0.8f * dt;
    if (Input_IsKeyDown('E')) g_zoomTarget += 0.8f * dt;
    if (g_zoomTarget < 0.2f) g_zoomTarget = 0.2f;
    if (g_zoomTarget > 4.0f) g_zoomTarget = 4.0f;
    // Lerp suave do zoom
    float zoomCurrent = g_cam.zoom;
    float zoomDelta   = (g_zoomTarget - zoomCurrent) * 8.0f * dt;
    Camera_SetZoom(&g_cam, zoomCurrent + zoomDelta);

    // --- Shake ---
    if (Input_WasKeyPressed('F'))
        Camera_Shake(&g_cam, 12.0f, 0.4f);

    // --- Update NPCs com bounce nos limites do mundo ---
    for (int i = 1; i < g_entities.count; i++)
    {
        IXEntity* e = &g_entities.entities[i];
        Entity_Update(e, dt);
        if (e->x < 0.0f)              { e->x = 0.0f;                    e->vx = -e->vx; }
        if (e->y < 0.0f)              { e->y = 0.0f;                    e->vy = -e->vy; }
        if (e->x + e->w > WORLD_W)    { e->x = (float)(WORLD_W - e->w); e->vx = -e->vx; }
        if (e->y + e->h > WORLD_H)    { e->y = (float)(WORLD_H - e->h); e->vy = -e->vy; }
    }
    Entity_Update(player, dt);

    // --- Camera segue o player ---
    Camera_Follow(&g_cam, player->x + player->w * 0.5f,
                          player->y + player->h * 0.5f, dt);
    Camera_Update(&g_cam, dt);

    // --- Converte mouse para coordenadas de mundo ---
    float worldMouseX, worldMouseY;
    Camera_ScreenToWorld(&g_cam, mouseX, mouseY, &worldMouseX, &worldMouseY);

    // Conta quantas entidades estao visiveis (culling)
    int visible = 0;
    for (int i = 0; i < g_entities.count; i++)
    {
        IXEntity* e = &g_entities.entities[i];
        if (Camera_IsVisible(&g_cam, e->x, e->y, e->w, e->h)) visible++;
    }

    std::snprintf(g_statusLine, sizeof(g_statusLine),
        "Player: %.0f,%.0f | Cam: %.0f,%.0f | Zoom: %.2fx | Visiveis: %d/%d | Mouse mundo: %.0f,%.0f",
        player->x, player->y,
        g_cam.x, g_cam.y,
        g_cam.zoom,
        visible, g_entities.count,
        worldMouseX, worldMouseY);
}

void DebugCameraTest_Render()
{
    // Grade do mundo (sem camera — usa WorldToScreen manual)
    DrawWorldGrid();

    // Entidades com camera attached (Entity_Render aplica transformacao)
    Camera_Attach(&g_cam);
    EntityList_RenderAll(&g_entities);
    Camera_Detach();
}

void DebugCameraTest_ApplyRendererState(float renderFPS, float frameMs)
{
    Renderer_SetOverlayText("InteractiveX r0.2", "Camera test", g_statusLine);
    Renderer_SetHelpText(g_helpLine);
    Renderer_SetDisplayOptions(1, 1, 0);
    Renderer_SetControlButton(NULL);
    Renderer_SetControlButtons(NULL, 0);
    Renderer_SetControlSliders(NULL, 0);
    Renderer_SetOverlayMetrics(renderFPS, frameMs, 0.0f);
}

void DebugCameraTest_HandleKeyDown(unsigned int key)
{
    if (key == VK_ESCAPE) g_returnToMenu = 1;
    if (key == 'F') Camera_Shake(&g_cam, 12.0f, 0.4f);
}

int  DebugCameraTest_ShouldReturnToMenu() { return g_returnToMenu; }
void DebugCameraTest_ClearReturnToMenu()  { g_returnToMenu = 0;    }
