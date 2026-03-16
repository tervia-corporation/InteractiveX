#ifndef IX_CORE_TILEMAP_H
#define IX_CORE_TILEMAP_H

#ifdef __cplusplus
extern "C" {
#endif

struct IXEntity;

// ---------------------------------------------------------------------------
//  Limites do mapa (armazenados no proprio struct, sem alocacao dinamica)
// ---------------------------------------------------------------------------
#define IX_TILEMAP_MAX_COLS  128
#define IX_TILEMAP_MAX_ROWS   64
#define IX_TILE_TYPE_COUNT   256  // tipos 0..255

// ---------------------------------------------------------------------------
//  IXTilemap
//
//  Grade de tiles com render integrado a camera e resolucao de colisao AABB.
//
//  Tile type 0 = vazio (transparente, sem colisao).
//  Tipos 1-255 = solido por padrao. Use Tilemap_SetSolid para customizar.
//  Cada tipo tem uma cor ARGB configuravel via Tilemap_SetTileColor.
//
//  Uso tipico:
//
//      IXTilemap map;
//      Tilemap_Init(&map, 40, 20, 32, 32);
//      Tilemap_SetTileColor(&map, 1, 0xFF607040);  // grama
//      Tilemap_SetTileColor(&map, 2, 0xFF808090);  // pedra
//
//      Tilemap_Fill(&map, 0, 19, 40, 1, 1);        // chao
//      Tilemap_SetTile(&map, 5, 15, 2);             // bloco de pedra
//
//      // a cada frame:
//      Camera_Attach(&cam);
//      Tilemap_Render(&map);
//      EntityList_RenderAll(&entities);
//      Camera_Detach();
//
//      Tilemap_ResolveCollision(&map, &player);
// ---------------------------------------------------------------------------
typedef struct IXTilemap
{
    int cols, rows;     // dimensoes em tiles
    int tileW, tileH;   // tamanho de cada tile em pixels

    // Grid de tipos: tiles[row * IX_TILEMAP_MAX_COLS + col]
    unsigned char tiles[IX_TILEMAP_MAX_ROWS * IX_TILEMAP_MAX_COLS];

    // Cor ARGB de cada tipo (indice = tipo, 0 = vazio/ignorado)
    unsigned int  tileColors[IX_TILE_TYPE_COUNT];

    // Flags de solidez por tipo (1 = solido, 0 = passavel)
    unsigned char tileSolid[IX_TILE_TYPE_COUNT];

} IXTilemap;

// ---------------------------------------------------------------------------
//  Inicializacao
// ---------------------------------------------------------------------------
// Inicializa o mapa. Limpa todos os tiles (tipo 0).
// Tipos 1-255 ficam solidos e com cores padrao.
void Tilemap_Init(IXTilemap* map, int cols, int rows, int tileW, int tileH);

// ---------------------------------------------------------------------------
//  Configuracao de tipos
// ---------------------------------------------------------------------------
// Define a cor ARGB de um tipo de tile.
void Tilemap_SetTileColor(IXTilemap* map, unsigned char type, unsigned int color);

// Define se um tipo e solido (afeta colisao). Padrao: tipos 1-255 sao solidos.
void Tilemap_SetSolid(IXTilemap* map, unsigned char type, int solid);

// ---------------------------------------------------------------------------
//  Edicao de tiles
// ---------------------------------------------------------------------------
void          Tilemap_SetTile(IXTilemap* map, int col, int row, unsigned char type);
unsigned char Tilemap_GetTile(const IXTilemap* map, int col, int row);

// Preenche um retangulo de tiles com o tipo dado.
void Tilemap_Fill(IXTilemap* map,
                  int col, int row, int cols, int rows,
                  unsigned char type);

// Limpa todos os tiles (preenche com tipo 0).
void Tilemap_Clear(IXTilemap* map);

// ---------------------------------------------------------------------------
//  Queries
// ---------------------------------------------------------------------------
int Tilemap_GetWorldWidth(const IXTilemap* map);
int Tilemap_GetWorldHeight(const IXTilemap* map);

// Converte posicao de mundo para coordenada de tile.
void Tilemap_WorldToTile(const IXTilemap* map,
                          float worldX, float worldY,
                          int* col, int* row);

// Retorna 1 se o tile na coluna/linha e solido.
int Tilemap_IsSolid(const IXTilemap* map, int col, int row);

// Retorna 1 se algum tile solido sobrepoe o retangulo de mundo dado.
int Tilemap_OverlapsSolid(const IXTilemap* map,
                           float x, float y, int w, int h);

// ---------------------------------------------------------------------------
//  Rendering
//
//  Renderiza apenas os tiles visiveis no viewport atual.
//  Se Camera_GetActive() != NULL, usa WorldToScreen para posicionar os tiles.
//  Se nenhuma camera estiver attached, usa coordenadas de mundo diretamente.
// ---------------------------------------------------------------------------
void Tilemap_Render(const IXTilemap* map);

// ---------------------------------------------------------------------------
//  Colisao
//
//  Resolve colisao AABB entre a entidade e os tiles solidos do mapa.
//  Move a entidade para fora dos tiles que ela esta penetrando.
//  Zera a velocidade no eixo de colisao (entity->vx ou vy = 0).
//
//  Deve ser chamado APOS Entity_Update() no mesmo frame.
// ---------------------------------------------------------------------------
void Tilemap_ResolveCollision(const IXTilemap* map, IXEntity* entity);

#ifdef __cplusplus
}
#endif

#endif // IX_CORE_TILEMAP_H
