#include "entity.h"
#include "camera.h"
#include "../renderer.h"
#include "../gui.h"
#include <cstring>

// ---------------------------------------------------------------------------
//  IXEntity
// ---------------------------------------------------------------------------

void Entity_Init(IXEntity* e, float x, float y, int w, int h)
{
    if (!e) return;
    std::memset(e, 0, sizeof(IXEntity));
    e->x       = x;
    e->y       = y;
    e->w       = w;
    e->h       = h;
    e->tint    = 0xFFFFFFFF;
    e->active  = 1;
    e->visible = 1;
}

void Entity_SetTexture(IXEntity* e, IDirect3DTexture9* texture)
{
    if (!e) return;
    e->texture = texture;
}

void Entity_SetSpriteFrame(IXEntity* e, int srcX, int srcY, int srcW, int srcH)
{
    if (!e) return;
    e->srcX = srcX; e->srcY = srcY;
    e->srcW = srcW; e->srcH = srcH;
}

void Entity_SetPosition(IXEntity* e, float x, float y)
{
    if (!e) return;
    e->x = x; e->y = y;
}

void Entity_SetVelocity(IXEntity* e, float vx, float vy)
{
    if (!e) return;
    e->vx = vx; e->vy = vy;
}

void Entity_SetSize(IXEntity* e, int w, int h)
{
    if (!e) return;
    e->w = w; e->h = h;
}

void Entity_SetTint(IXEntity* e, unsigned int tint)
{
    if (!e) return;
    e->tint = tint;
}

void Entity_SetActive(IXEntity* e, int active)
{
    if (!e) return;
    e->active = active ? 1 : 0;
}

void Entity_SetVisible(IXEntity* e, int visible)
{
    if (!e) return;
    e->visible = visible ? 1 : 0;
}

void Entity_GetBounds(const IXEntity* e, int* x, int* y, int* w, int* h)
{
    if (!e) return;
    if (x) *x = (int)e->x;
    if (y) *y = (int)e->y;
    if (w) *w = e->w;
    if (h) *h = e->h;
}

void Entity_Update(IXEntity* e, float deltaSeconds)
{
    if (!e || !e->active) return;
    e->x += e->vx * deltaSeconds;
    e->y += e->vy * deltaSeconds;
}

int Entity_Overlaps(const IXEntity* a, const IXEntity* b)
{
    if (!a || !b || !a->active || !b->active) return 0;
    int ax = (int)a->x, ay = (int)a->y;
    int bx = (int)b->x, by = (int)b->y;
    if (ax + a->w <= bx || bx + b->w <= ax) return 0;
    if (ay + a->h <= by || by + b->h <= ay) return 0;
    return 1;
}

int Entity_OverlapsCircle(const IXEntity* a, const IXEntity* b)
{
    if (!a || !b || !a->active || !b->active) return 0;
    float acx = a->x + (float)a->w * 0.5f;
    float acy = a->y + (float)a->h * 0.5f;
    float bcx = b->x + (float)b->w * 0.5f;
    float bcy = b->y + (float)b->h * 0.5f;
    float ar  = (float)(a->w < a->h ? a->w : a->h) * 0.5f;
    float br  = (float)(b->w < b->h ? b->w : b->h) * 0.5f;
    float dx  = bcx - acx;
    float dy  = bcy - acy;
    float r   = ar + br;
    return (dx * dx + dy * dy) < (r * r) ? 1 : 0;
}

int Entity_ContainsPoint(const IXEntity* e, float px, float py)
{
    if (!e || !e->active) return 0;
    return (px >= e->x && px < e->x + (float)e->w &&
            py >= e->y && py < e->y + (float)e->h) ? 1 : 0;
}

int Entity_RayHit(const IXEntity* e,
                  float ox, float oy,
                  float dx, float dy,
                  float maxLen, float* tHit)
{
    if (!e || !e->active) return 0;

    float rx = e->x, ry = e->y;
    float rw = (float)e->w, rh = (float)e->h;
    float tmin = 0.0f, tmax = maxLen;

    // Eixo X
    if (dx != 0.0f)
    {
        float t1 = (rx      - ox) / dx;
        float t2 = (rx + rw - ox) / dx;
        if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
        if (t1 > tmin) tmin = t1;
        if (t2 < tmax) tmax = t2;
    }
    else if (ox < rx || ox > rx + rw) return 0;

    // Eixo Y
    if (dy != 0.0f)
    {
        float t1 = (ry      - oy) / dy;
        float t2 = (ry + rh - oy) / dy;
        if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
        if (t1 > tmin) tmin = t1;
        if (t2 < tmax) tmax = t2;
    }
    else if (oy < ry || oy > ry + rh) return 0;

    if (tmax < tmin) return 0;
    if (tHit) *tHit = tmin;
    return 1;
}

void Entity_Render(const IXEntity* e)
{
    if (!e || !e->active || !e->visible) return;

    // Se ha uma camera attached, transforma coordenadas de mundo -> tela
    // e aplica zoom ao tamanho renderizado.
    int dx = (int)e->x;
    int dy = (int)e->y;
    int dw = e->w;
    int dh = e->h;

    IXCamera* cam = Camera_GetActive();
    if (cam)
    {
        // Culling: nao renderiza se fora do viewport
        if (!Camera_IsVisible(cam, e->x, e->y, e->w, e->h)) return;

        Camera_WorldToScreen(cam, e->x, e->y, &dx, &dy);

        // Escala o tamanho pelo zoom
        float zoom = cam->zoom > 0.0f ? cam->zoom : 1.0f;
        dw = (int)((float)e->w * zoom);
        dh = (int)((float)e->h * zoom);
        if (dw < 1) dw = 1;
        if (dh < 1) dh = 1;
    }

    if (e->texture)
    {
        Renderer_DrawSprite(e->texture,
            e->srcX, e->srcY, e->srcW, e->srcH,
            dx, dy, dw, dh,
            e->tint);
    }
    else
    {
        GUI_DrawFilledRect(dx, dy, dw, dh, e->tint);
    }
}

// ---------------------------------------------------------------------------
//  IXEntityList
// ---------------------------------------------------------------------------

void EntityList_Init(IXEntityList* list, IXEntity* buffer, int capacity)
{
    if (!list) return;
    list->entities = buffer;
    list->capacity = capacity;
    list->count    = 0;
    if (buffer && capacity > 0)
        std::memset(buffer, 0, sizeof(IXEntity) * (size_t)capacity);
}

int EntityList_Add(IXEntityList* list)
{
    if (!list || list->count >= list->capacity) return -1;
    int idx = list->count;
    Entity_Init(&list->entities[idx], 0.0f, 0.0f, 32, 32);
    list->count++;
    return idx;
}

void EntityList_Remove(IXEntityList* list, int index)
{
    if (!list || index < 0 || index >= list->count) return;
    int last = list->count - 1;
    if (index != last) list->entities[index] = list->entities[last];
    list->count--;
}

void EntityList_Clear(IXEntityList* list)
{
    if (!list) return;
    list->count = 0;
}

void EntityList_UpdateAll(IXEntityList* list, float deltaSeconds)
{
    if (!list) return;
    for (int i = 0; i < list->count; i++)
        Entity_Update(&list->entities[i], deltaSeconds);
}

void EntityList_RenderAll(const IXEntityList* list)
{
    if (!list) return;
    for (int i = 0; i < list->count; i++)
        Entity_Render(&list->entities[i]);
}

int EntityList_GetCount(const IXEntityList* list)
{
    return list ? list->count : 0;
}
