#include "tilemap_test.h"
#include "../core/tilemap.h"
#include "../core/entity.h"
#include "../core/camera.h"
#include "../core/input.h"
#include "../core/timer.h"
#include "../renderer.h"
#include <cstdio>
#include <cstring>

// ---------------------------------------------------------------------------
//  Configuracao
// ---------------------------------------------------------------------------
#define MAP_COLS       60
#define MAP_ROWS       24
#define TILE_SIZE      32
#define PLAYER_W       28
#define PLAYER_H       30
#define PLAYER_SPEED  200.0f
#define JUMP_FORCE   -480.0f
#define GRAVITY       900.0f
#define MAX_FALL      600.0f

#define TILE_GRAMA   1
#define TILE_PEDRA   2
#define TILE_TERRA   4
#define TILE_MADEIRA 6
#define TILE_AGUA    3

// ---------------------------------------------------------------------------
//  Estado
// ---------------------------------------------------------------------------
static IXTilemap  g_map;
static IXCamera   g_cam;
static IXEntity   g_player;
static int        g_onGround     = 0;
static int        g_returnToMenu = 0;
static char       g_statusLine[256] = "Tilemap test ready.";
static char       g_helpLine[128]   = "AD mover | W/SPACE pular | ESC menu";

// ---------------------------------------------------------------------------
//  Detecta se o player esta no chao verificando tiles diretamente abaixo
//  (1 pixel abaixo dos pes, varrendo toda a largura do player)
// ---------------------------------------------------------------------------
static int ProbeGround()
{
    // Ponto logo abaixo do player (1 pixel abaixo dos pes)
    float feetY = g_player.y + (float)g_player.h + 1.0f;
    float leftX = g_player.x + 1.0f;             // margem interna de 1px
    float rightX = g_player.x + (float)g_player.w - 2.0f;

    int row = (int)(feetY / (float)TILE_SIZE);
    int c0  = (int)(leftX  / (float)TILE_SIZE);
    int c1  = (int)(rightX / (float)TILE_SIZE);

    for (int c = c0; c <= c1; c++)
        if (Tilemap_IsSolid(&g_map, c, row)) return 1;
    return 0;
}

// ---------------------------------------------------------------------------
//  Monta mapa de demonstracao
// ---------------------------------------------------------------------------
static void BuildMap()
{
    Tilemap_Clear(&g_map);

    Tilemap_Fill(&g_map, 0, MAP_ROWS - 1, MAP_COLS, 1, TILE_GRAMA);
    Tilemap_Fill(&g_map, 0, MAP_ROWS - 3, MAP_COLS, 2, TILE_TERRA);
    Tilemap_Fill(&g_map, 0,           0, 1, MAP_ROWS, TILE_PEDRA);
    Tilemap_Fill(&g_map, MAP_COLS - 1, 0, 1, MAP_ROWS, TILE_PEDRA);
    Tilemap_Fill(&g_map, 0, 0, MAP_COLS, 1, TILE_PEDRA);

    // Plataformas
    Tilemap_Fill(&g_map,  4, MAP_ROWS - 6,  8, 1, TILE_MADEIRA);
    Tilemap_Fill(&g_map, 14, MAP_ROWS - 9,  7, 1, TILE_MADEIRA);
    Tilemap_Fill(&g_map, 24, MAP_ROWS - 13, 6, 1, TILE_PEDRA);
    for (int i = 0; i < 5; i++)
        Tilemap_Fill(&g_map, 32 + i, MAP_ROWS - 5 - i, 1, 1, TILE_PEDRA);
    Tilemap_Fill(&g_map, 44, MAP_ROWS - 10, 10, 1, TILE_MADEIRA);
    Tilemap_Fill(&g_map, 20, MAP_ROWS - 4,   3, 2, TILE_PEDRA);

    // Agua decorativa (nao solida)
    Tilemap_Fill(&g_map, 7, MAP_ROWS - 2, 5, 1, TILE_AGUA);
    Tilemap_SetSolid(&g_map, TILE_AGUA, 0);

    // Buraco
    Tilemap_Fill(&g_map, 7, MAP_ROWS - 1, 5, 1, 0);
    Tilemap_Fill(&g_map, 7, MAP_ROWS - 3, 5, 2, 0);
}

// ---------------------------------------------------------------------------
//  Public API
// ---------------------------------------------------------------------------
void DebugTilemapTest_Initialize()
{
    g_returnToMenu = 0;
    g_onGround     = 0;

    Tilemap_Init(&g_map, MAP_COLS, MAP_ROWS, TILE_SIZE, TILE_SIZE);
    Tilemap_SetTileColor(&g_map, TILE_AGUA, 0xAA3868A8);
    BuildMap();

    Camera_Init(&g_cam);
    Camera_SetWorldBounds(&g_cam, 0.0f, 0.0f,
        (float)Tilemap_GetWorldWidth(&g_map),
        (float)Tilemap_GetWorldHeight(&g_map));
    Camera_SetFollowLerp(&g_cam, 0.10f);

    std::memset(&g_player, 0, sizeof(g_player));
    Entity_Init(&g_player,
        (float)(TILE_SIZE * 2),
        (float)(TILE_SIZE * (MAP_ROWS - 4) - PLAYER_H),
        PLAYER_W, PLAYER_H);
    Entity_SetTint(&g_player, 0xFFFFFFFF);

    Camera_CenterOn(&g_cam, g_player.x, g_player.y);
    Camera_Update(&g_cam, 0.0f);
}

void DebugTilemapTest_InvalidateLayout() {}

void DebugTilemapTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased)
{
    (void)mouseX; (void)mouseY; (void)mouseDown; (void)mouseReleased;

    if (Input_WasKeyPressed(VK_ESCAPE)) { g_returnToMenu = 1; return; }

    float dt = Timer_GetDeltaSeconds();
    if (dt > 0.05f) dt = 0.05f;

    // --- Chao via probe (antes do input para pulo) ---
    g_onGround = ProbeGround();

    // --- Input horizontal ---
    g_player.vx = 0.0f;
    if (Input_IsKeyDown('A') || Input_IsKeyDown(VK_LEFT))  g_player.vx = -PLAYER_SPEED;
    if (Input_IsKeyDown('D') || Input_IsKeyDown(VK_RIGHT)) g_player.vx =  PLAYER_SPEED;

    // --- Pulo: so W e SPACE (VK_UP reservado para movimento futuro) ---
    if (g_onGround && (Input_WasKeyPressed(VK_SPACE) || Input_WasKeyPressed('W')))
    {
        g_player.vy = JUMP_FORCE;
        g_onGround  = 0;
    }

    // --- Gravidade ---
    g_player.vy += GRAVITY * dt;
    if (g_player.vy > MAX_FALL) g_player.vy = MAX_FALL;

    // --- Move e resolve colisao ---
    Entity_Update(&g_player, dt);
    Tilemap_ResolveCollision(&g_map, &g_player);

    // --- Camera ---
    Camera_Follow(&g_cam,
        g_player.x + g_player.w * 0.5f,
        g_player.y + g_player.h * 0.5f, dt);
    Camera_Update(&g_cam, dt);

    // Status
    int tileCol, tileRow;
    Tilemap_WorldToTile(&g_map,
        g_player.x + g_player.w * 0.5f,
        g_player.y + g_player.h,
        &tileCol, &tileRow);
    unsigned char tileUnder = Tilemap_GetTile(&g_map, tileCol, tileRow);

    std::snprintf(g_statusLine, sizeof(g_statusLine),
        "Player: %.0f,%.0f | vy: %.0f | %s | Tile sob: %d (%d,%d)",
        g_player.x, g_player.y, g_player.vy,
        g_onGround ? "chao" : "ar",
        (int)tileUnder, tileCol, tileRow);
}

void DebugTilemapTest_Render()
{
    Camera_Attach(&g_cam);
    Tilemap_Render(&g_map);
    Entity_Render(&g_player);
    Camera_Detach();
}

void DebugTilemapTest_ApplyRendererState(float renderFPS, float frameMs)
{
    Renderer_SetOverlayText("InteractiveX r0.2", "Tilemap test", g_statusLine);
    Renderer_SetHelpText(g_helpLine);
    Renderer_SetDisplayOptions(1, 1, 0);
    Renderer_SetControlButton(NULL);
    Renderer_SetControlButtons(NULL, 0);
    Renderer_SetControlSliders(NULL, 0);
    Renderer_SetOverlayMetrics(renderFPS, frameMs, 0.0f);
}

void DebugTilemapTest_HandleKeyDown(unsigned int key)
{
    if (key == VK_ESCAPE) g_returnToMenu = 1;
}

int  DebugTilemapTest_ShouldReturnToMenu() { return g_returnToMenu; }
void DebugTilemapTest_ClearReturnToMenu()  { g_returnToMenu = 0;    }
