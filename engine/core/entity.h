#ifndef IX_CORE_ENTITY_H
#define IX_CORE_ENTITY_H

#ifdef __cplusplus
extern "C" {
#endif

struct IDirect3DTexture9;

// ---------------------------------------------------------------------------
//  IXEntity
//
//  Unidade basica de jogo: posicao, velocidade, tamanho, textura e estado.
//
//  Rendering:
//    - Se texture != NULL, chama Renderer_DrawSprite com os UVs definidos.
//    - Se texture == NULL, desenha um retangulo solido usando 'tint' como cor.
//    - srcW == 0 && srcH == 0  ->  usa a textura inteira (sem recorte).
//
//  Uso tipico:
//      IXEntity e;
//      Entity_Init(&e, 100.0f, 200.0f, 64, 64);
//      Entity_SetTexture(&e, img.texture);
//      Entity_SetVelocity(&e, 80.0f, 0.0f);   // 80 px/s para direita
//
//      // a cada frame:
//      Entity_Update(&e, Timer_GetDeltaSeconds());
//      Entity_Render(&e);
// ---------------------------------------------------------------------------
typedef struct IXEntity
{
    // --- Transform ---
    float x, y;         // posicao top-left (pixels, ponto flutuante)
    float vx, vy;       // velocidade (pixels por segundo)
    int   w, h;         // tamanho em pixels

    // --- Rendering ---
    IDirect3DTexture9* texture;     // NULL = retangulo solido
    int          srcX, srcY;        // recorte na spritesheet
    int          srcW, srcH;        // 0,0 = textura inteira
    unsigned int tint;              // ARGB. sem textura: cor do retangulo
                                    // com textura: multiplicador de cor
                                    // 0xFFFFFFFF = sem alteracao

    // --- Estado ---
    int active;         // 0 = ignorado por Update e Render
    int visible;        // 0 = ignorado apenas por Render
} IXEntity;

// --- Inicializacao ---
void Entity_Init(IXEntity* e, float x, float y, int w, int h);

// --- Configuracao ---
void Entity_SetTexture(IXEntity* e, IDirect3DTexture9* texture);
void Entity_SetSpriteFrame(IXEntity* e, int srcX, int srcY, int srcW, int srcH);
void Entity_SetPosition(IXEntity* e, float x, float y);
void Entity_SetVelocity(IXEntity* e, float vx, float vy);
void Entity_SetSize(IXEntity* e, int w, int h);
void Entity_SetTint(IXEntity* e, unsigned int tint);
void Entity_SetActive(IXEntity* e, int active);
void Entity_SetVisible(IXEntity* e, int visible);

// --- Leitura ---
void Entity_GetBounds(const IXEntity* e, int* x, int* y, int* w, int* h);

// --- Logica ---

// Aplica velocidade a posicao: x += vx * delta, y += vy * delta.
void Entity_Update(IXEntity* e, float deltaSeconds);

// Retorna 1 se os bounding boxes de 'a' e 'b' se sobrepoem (AABB).
int  Entity_Overlaps(const IXEntity* a, const IXEntity* b);

// Retorna 1 se os circulos de 'a' e 'b' se sobrepoem.
// O circulo de cada entidade tem centro em (x + w/2, y + h/2)
// e raio = min(w, h) / 2.
int  Entity_OverlapsCircle(const IXEntity* a, const IXEntity* b);

// Retorna 1 se o ponto (px, py) esta dentro do bounding box de 'e'.
int  Entity_ContainsPoint(const IXEntity* e, float px, float py);

// Ray vs AABB: retorna 1 se o raio acerta o bounding box de 'e'.
// ox, oy    : origem do raio
// dx, dy    : direcao normalizada do raio
// maxLen    : comprimento maximo do raio
// tHit      : se nao NULL, recebe a distancia ao ponto de impacto (0..maxLen)
int  Entity_RayHit(const IXEntity* e,
                   float ox, float oy,
                   float dx, float dy,
                   float maxLen, float* tHit);

// --- Rendering ---

// Chama Renderer_DrawSprite (com textura) ou GUI_DrawFilledRect (sem textura).
// Deve ser chamado entre Renderer_BeginFrame() e Renderer_EndFrame().
void Entity_Render(const IXEntity* e);

// ---------------------------------------------------------------------------
//  IXEntityList
//
//  Lista de entidades com buffer de tamanho fixo fornecido pelo usuario.
//  Remove = swap com o ultimo (sem gaps, ordem nao garantida).
//
//  Uso tipico:
//      IXEntity buf[64];
//      IXEntityList list;
//      EntityList_Init(&list, buf, 64);
//
//      int idx = EntityList_Add(&list);
//      if (idx >= 0) Entity_SetVelocity(&list.entities[idx], 50.0f, 0.0f);
//
//      EntityList_UpdateAll(&list, Timer_GetDeltaSeconds());
//      EntityList_RenderAll(&list);
// ---------------------------------------------------------------------------
typedef struct IXEntityList
{
    IXEntity* entities;   // buffer fornecido pelo usuario
    int       capacity;   // tamanho do buffer
    int       count;      // numero de entidades ativas
} IXEntityList;

void EntityList_Init(IXEntityList* list, IXEntity* buffer, int capacity);

// Adiciona uma entidade default-inicializada e retorna seu indice, ou -1 se cheio.
int  EntityList_Add(IXEntityList* list);

// Remove a entidade no indice dado (swap com ultima).
void EntityList_Remove(IXEntityList* list, int index);

// Remove todas as entidades.
void EntityList_Clear(IXEntityList* list);

// Chama Entity_Update em todas as entidades ativas.
void EntityList_UpdateAll(IXEntityList* list, float deltaSeconds);

// Chama Entity_Render em todas as entidades visiveis e ativas.
void EntityList_RenderAll(const IXEntityList* list);

// Retorna o numero de entidades ativas.
int  EntityList_GetCount(const IXEntityList* list);

#ifdef __cplusplus
}
#endif

#endif // IX_CORE_ENTITY_H
