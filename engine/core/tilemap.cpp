#include "tilemap.h"
#include "entity.h"
#include "camera.h"
#include "../renderer.h"
#include "../gui.h"
#include <cstring>

// ---------------------------------------------------------------------------
//  Cores padrao para tipos 1-15
// ---------------------------------------------------------------------------
static const unsigned int DEFAULT_COLORS[16] = {
    0x00000000,  // 0 = vazio
    0xFF607048,  // 1 = grama
    0xFF808090,  // 2 = pedra
    0xFF5878A8,  // 3 = agua
    0xFFA07830,  // 4 = terra
    0xFFD8C880,  // 5 = areia
    0xFF503828,  // 6 = madeira
    0xFF486830,  // 7 = folhagem
    0xFFC04030,  // 8 = lava
    0xFF88A8C8,  // 9 = gelo
    0xFF404858,  // 10 = metal
    0xFFB8B060,  // 11 = ouro
    0xFF784088,  // 12 = cristal
    0xFF303840,  // 13 = sombra
    0xFFE8E8E8,  // 14 = neve
    0xFF204020,  // 15 = musgo
};

// ---------------------------------------------------------------------------
//  Clamp interno
// ---------------------------------------------------------------------------
static int Clampi(int v, int lo, int hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}
static float Clampf(float v, float lo, float hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

// ---------------------------------------------------------------------------
//  Inicializacao
// ---------------------------------------------------------------------------
void Tilemap_Init(IXTilemap* map, int cols, int rows, int tileW, int tileH)
{
    if (!map) return;
    std::memset(map, 0, sizeof(IXTilemap));

    map->cols  = Clampi(cols,  1, IX_TILEMAP_MAX_COLS);
    map->rows  = Clampi(rows,  1, IX_TILEMAP_MAX_ROWS);
    map->tileW = tileW > 1 ? tileW : 1;
    map->tileH = tileH > 1 ? tileH : 1;

    // Cores padrao
    for (int i = 0; i < IX_TILE_TYPE_COUNT; i++)
    {
        if (i < 16)
            map->tileColors[i] = DEFAULT_COLORS[i];
        else
        {
            // Gera uma cor unica para tipos > 15 (variacao de cinza-azulado)
            unsigned int v = 0x40 + (unsigned int)(i * 7 % 0x80);
            map->tileColors[i] = 0xFF000000 | (v << 16) | (v << 8) | v;
        }
        // Tipo 0 = vazio (nao solido), 1-255 = solido por padrao
        map->tileSolid[i] = (i > 0) ? 1 : 0;
    }
}

// ---------------------------------------------------------------------------
//  Configuracao de tipos
// ---------------------------------------------------------------------------
void Tilemap_SetTileColor(IXTilemap* map, unsigned char type, unsigned int color)
{
    if (!map) return;
    map->tileColors[type] = color;
}

void Tilemap_SetSolid(IXTilemap* map, unsigned char type, int solid)
{
    if (!map) return;
    map->tileSolid[type] = solid ? 1 : 0;
}

// ---------------------------------------------------------------------------
//  Edicao
// ---------------------------------------------------------------------------
void Tilemap_SetTile(IXTilemap* map, int col, int row, unsigned char type)
{
    if (!map || col < 0 || col >= map->cols || row < 0 || row >= map->rows) return;
    map->tiles[row * IX_TILEMAP_MAX_COLS + col] = type;
}

unsigned char Tilemap_GetTile(const IXTilemap* map, int col, int row)
{
    if (!map || col < 0 || col >= map->cols || row < 0 || row >= map->rows) return 0;
    return map->tiles[row * IX_TILEMAP_MAX_COLS + col];
}

void Tilemap_Fill(IXTilemap* map,
                  int col, int row, int cols, int rows,
                  unsigned char type)
{
    if (!map) return;
    for (int r = row; r < row + rows; r++)
        for (int c = col; c < col + cols; c++)
            Tilemap_SetTile(map, c, r, type);
}

void Tilemap_Clear(IXTilemap* map)
{
    if (!map) return;
    std::memset(map->tiles, 0, sizeof(map->tiles));
}

// ---------------------------------------------------------------------------
//  Queries
// ---------------------------------------------------------------------------
int Tilemap_GetWorldWidth(const IXTilemap* map)
{
    return map ? map->cols * map->tileW : 0;
}

int Tilemap_GetWorldHeight(const IXTilemap* map)
{
    return map ? map->rows * map->tileH : 0;
}

void Tilemap_WorldToTile(const IXTilemap* map,
                          float worldX, float worldY,
                          int* col, int* row)
{
    if (!map) return;
    if (col) *col = (int)(worldX / (float)map->tileW);
    if (row) *row = (int)(worldY / (float)map->tileH);
}

int Tilemap_IsSolid(const IXTilemap* map, int col, int row)
{
    if (!map || col < 0 || col >= map->cols || row < 0 || row >= map->rows) return 0;
    unsigned char type = map->tiles[row * IX_TILEMAP_MAX_COLS + col];
    return map->tileSolid[type] ? 1 : 0;
}

int Tilemap_OverlapsSolid(const IXTilemap* map,
                           float x, float y, int w, int h)
{
    if (!map || w <= 0 || h <= 0) return 0;
    int c0 = (int)(x                    / (float)map->tileW);
    int c1 = (int)((x + (float)w - 1.0f) / (float)map->tileW);
    int r0 = (int)(y                    / (float)map->tileH);
    int r1 = (int)((y + (float)h - 1.0f) / (float)map->tileH);
    for (int r = r0; r <= r1; r++)
        for (int c = c0; c <= c1; c++)
            if (Tilemap_IsSolid(map, c, r)) return 1;
    return 0;
}

// ---------------------------------------------------------------------------
//  Rendering
// ---------------------------------------------------------------------------
void Tilemap_Render(const IXTilemap* map)
{
    if (!map) return;

    IXCamera* cam = Camera_GetActive();

    // Determina quais tiles estao visiveis
    int c0 = 0, r0 = 0, c1 = map->cols - 1, r1 = map->rows - 1;

    if (cam)
    {
        float zoom = cam->zoom > 0.0f ? cam->zoom : 1.0f;
        // Tile de inicio (com margem de 1)
        c0 = (int)(cam->x / (float)map->tileW) - 1;
        r0 = (int)(cam->y / (float)map->tileH) - 1;
        // Tile de fim
        c1 = (int)((cam->x + (float)cam->viewportW / zoom) / (float)map->tileW) + 1;
        r1 = (int)((cam->y + (float)cam->viewportH / zoom) / (float)map->tileH) + 1;
        // Clamp
        if (c0 < 0) c0 = 0;
        if (r0 < 0) r0 = 0;
        if (c1 >= map->cols) c1 = map->cols - 1;
        if (r1 >= map->rows) r1 = map->rows - 1;
    }

    for (int r = r0; r <= r1; r++)
    {
        for (int c = c0; c <= c1; c++)
        {
            unsigned char type = map->tiles[r * IX_TILEMAP_MAX_COLS + c];
            if (type == 0) continue;

            float wx = (float)(c * map->tileW);
            float wy = (float)(r * map->tileH);

            int sx = (int)wx, sy = (int)wy, sw = map->tileW, sh = map->tileH;

            if (cam)
            {
                Camera_WorldToScreen(cam, wx, wy, &sx, &sy);
                float zoom = cam->zoom > 0.0f ? cam->zoom : 1.0f;
                sw = (int)((float)map->tileW * zoom);
                sh = (int)((float)map->tileH * zoom);
                if (sw < 1) sw = 1;
                if (sh < 1) sh = 1;
            }

            unsigned int color = map->tileColors[type];
            GUI_DrawFilledRect(sx, sy, sw, sh, color);

            // Borda sutil entre tiles
            GUI_DrawOutlinedRect(sx, sy, sw, sh, 0x18000000);
        }
    }
}

// ---------------------------------------------------------------------------
//  Colisao AABB contra tiles solidos
//
//  Algoritmo: resolve X e Y separadamente para evitar corner-sticking.
//  1. Testa todos os tiles sobrepostos na nova posicao X.
//  2. Empurra para fora no eixo X se necessario.
//  3. Repete para Y.
// ---------------------------------------------------------------------------
void Tilemap_ResolveCollision(const IXTilemap* map, IXEntity* entity)
{
    if (!map || !entity || !entity->active) return;

    float x  = entity->x;
    float y  = entity->y;
    float x2 = x + (float)entity->w;
    float y2 = y + (float)entity->h;

    float tw = (float)map->tileW;
    float th = (float)map->tileH;

    // Tiles sobrepostos
    int c0 = (int)(x  / tw);     if (c0 < 0) c0 = 0;
    int c1 = (int)((x2 - 1.0f) / tw); if (c1 >= map->cols) c1 = map->cols - 1;
    int r0 = (int)(y  / th);     if (r0 < 0) r0 = 0;
    int r1 = (int)((y2 - 1.0f) / th); if (r1 >= map->rows) r1 = map->rows - 1;

    for (int r = r0; r <= r1; r++)
    {
        for (int c = c0; c <= c1; c++)
        {
            if (!Tilemap_IsSolid(map, c, r)) continue;

            float tx  = (float)(c * map->tileW);
            float ty  = (float)(r * map->tileH);
            float tx2 = tx + tw;
            float ty2 = ty + th;

            // Sobreposicao em cada eixo
            float overlapX = (x2 > tx && x < tx2) ?
                ((x2 - tx) < (tx2 - x) ? -(x2 - tx) : (tx2 - x)) : 0.0f;
            float overlapY = (y2 > ty && y < ty2) ?
                ((y2 - ty) < (ty2 - y) ? -(y2 - ty) : (ty2 - y)) : 0.0f;

            if (overlapX == 0.0f || overlapY == 0.0f) continue;

            // Resolve no eixo de menor penetracao
            float absX = overlapX < 0.0f ? -overlapX : overlapX;
            float absY = overlapY < 0.0f ? -overlapY : overlapY;

            if (absX < absY)
            {
                x += overlapX;
                x2 = x + (float)entity->w;
                entity->vx = 0.0f;
            }
            else
            {
                y += overlapY;
                y2 = y + (float)entity->h;
                entity->vy = 0.0f;
            }
        }
    }

    // Clamp nas bordas do mapa
    float mapW = (float)Tilemap_GetWorldWidth(map);
    float mapH = (float)Tilemap_GetWorldHeight(map);
    if (x < 0.0f)              { x = 0.0f;                       entity->vx = 0.0f; }
    if (y < 0.0f)              { y = 0.0f;                       entity->vy = 0.0f; }
    if (x + entity->w > mapW) { x = mapW - (float)entity->w;    entity->vx = 0.0f; }
    if (y + entity->h > mapH) { y = mapH - (float)entity->h;    entity->vy = 0.0f; }

    entity->x = x;
    entity->y = y;
}
