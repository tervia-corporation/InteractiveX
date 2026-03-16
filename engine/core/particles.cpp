#include "particles.h"
#include "camera.h"
#include "../renderer.h"
#include "../gui.h"
#include <cstring>
#include <cmath>

// ---------------------------------------------------------------------------
//  LCG sem <cstdlib> (determinístico, thread-unsafe mas suficiente)
// ---------------------------------------------------------------------------
static unsigned int g_rng = 0xDEADC0DE;

static float RandFloat(float lo, float hi)
{
    g_rng = g_rng * 1664525u + 1013904223u;
    float t = (float)(g_rng & 0xFFFF) / (float)0xFFFF;
    return lo + t * (hi - lo);
}

// ---------------------------------------------------------------------------
//  Interpolacao de cor ARGB  (t = 0..1)
// ---------------------------------------------------------------------------
static unsigned int LerpColor(unsigned int a, unsigned int b, float t)
{
    if (t <= 0.0f) return a;
    if (t >= 1.0f) return b;

    unsigned int aa = (a >> 24) & 0xFF;
    unsigned int ar = (a >> 16) & 0xFF;
    unsigned int ag = (a >>  8) & 0xFF;
    unsigned int ab = (a      ) & 0xFF;

    unsigned int ba = (b >> 24) & 0xFF;
    unsigned int br = (b >> 16) & 0xFF;
    unsigned int bg = (b >>  8) & 0xFF;
    unsigned int bb = (b      ) & 0xFF;

    unsigned int ra = (unsigned int)(aa + (ba - (int)aa) * t);
    unsigned int rr = (unsigned int)(ar + (br - (int)ar) * t);
    unsigned int rg = (unsigned int)(ag + (bg - (int)ag) * t);
    unsigned int rb = (unsigned int)(ab + (bb - (int)ab) * t);

    return (ra << 24) | (rr << 16) | (rg << 8) | rb;
}

// ---------------------------------------------------------------------------
//  Spawn de uma particula num slot disponivel
// ---------------------------------------------------------------------------
static void SpawnOne(IXEmitter* e)
{
    // Procura slot inativo
    for (int i = 0; i < e->capacity; i++)
    {
        IXParticle* p = &e->particles[i];
        if (p->active) continue;

        float life = RandFloat(e->lifeMin, e->lifeMax);
        if (life <= 0.0f) life = 0.01f;

        p->x       = e->x + RandFloat(-e->spreadX, e->spreadX);
        p->y       = e->y + RandFloat(-e->spreadY, e->spreadY);
        p->vx      = RandFloat(e->vxMin, e->vxMax);
        p->vy      = RandFloat(e->vyMin, e->vyMax);
        p->ax      = e->ax;
        p->ay      = e->ay;
        p->life    = life;
        p->lifeMax = life;
        p->size    = e->sizeStart;
        p->sizeEnd = e->sizeEnd;
        p->colorStart = e->colorStart;
        p->colorEnd   = e->colorEnd;
        p->active  = 1;
        e->activeCount++;
        return;
    }
    // Pool cheio — ignora silenciosamente
}

// ---------------------------------------------------------------------------
//  Inicializacao
// ---------------------------------------------------------------------------
void Emitter_Init(IXEmitter* e, IXParticle* buffer, int capacity)
{
    if (!e || !buffer || capacity <= 0) return;
    std::memset(e, 0, sizeof(IXEmitter));
    std::memset(buffer, 0, sizeof(IXParticle) * (size_t)capacity);

    e->particles   = buffer;
    e->capacity    = capacity;
    e->activeCount = 0;

    // Defaults razoaveis
    e->spawnRate  = 30.0f;
    e->lifeMin    = 0.5f;
    e->lifeMax    = 1.0f;
    e->vxMin      = -20.0f; e->vxMax = 20.0f;
    e->vyMin      = -80.0f; e->vyMax = -20.0f;
    e->ay         = 150.0f; // gravidade leve
    e->sizeStart  = 6;
    e->sizeEnd    = 1;
    e->colorStart = 0xFFFFAA33;
    e->colorEnd   = 0x00FF3300;
    e->active     = 1;
    e->paused     = 0;
}

// ---------------------------------------------------------------------------
//  Configuracao
// ---------------------------------------------------------------------------
void Emitter_SetPosition(IXEmitter* e, float x, float y)    { if (e) { e->x = x; e->y = y; } }
void Emitter_SetSpread(IXEmitter* e, float sx, float sy)    { if (e) { e->spreadX = sx; e->spreadY = sy; } }
void Emitter_SetSpawnRate(IXEmitter* e, float rate)         { if (e) e->spawnRate = rate; }
void Emitter_SetGravity(IXEmitter* e, float ax, float ay)   { if (e) { e->ax = ax; e->ay = ay; } }
void Emitter_SetSize(IXEmitter* e, int start, int end)      { if (e) { e->sizeStart = start; e->sizeEnd = end; } }

void Emitter_SetLifetime(IXEmitter* e, float minS, float maxS)
{
    if (!e) return;
    e->lifeMin = minS > 0.0f ? minS : 0.01f;
    e->lifeMax = maxS > e->lifeMin ? maxS : e->lifeMin;
}

void Emitter_SetVelocity(IXEmitter* e,
                          float vxMin, float vxMax,
                          float vyMin, float vyMax)
{
    if (!e) return;
    e->vxMin = vxMin; e->vxMax = vxMax;
    e->vyMin = vyMin; e->vyMax = vyMax;
}

void Emitter_SetColorOverLife(IXEmitter* e,
                               unsigned int colorStart,
                               unsigned int colorEnd)
{
    if (!e) return;
    e->colorStart = colorStart;
    e->colorEnd   = colorEnd;
}

void Emitter_SetTexture(IXEmitter* e, IDirect3DTexture9* texture,
                         int frameW, int frameH)
{
    if (!e) return;
    e->texture    = texture;
    e->texFrameW  = frameW;
    e->texFrameH  = frameH;
}

// ---------------------------------------------------------------------------
//  Controle
// ---------------------------------------------------------------------------
void Emitter_Burst(IXEmitter* e, int count)
{
    if (!e || count <= 0) return;
    for (int i = 0; i < count; i++)
        SpawnOne(e);
}

void Emitter_Stop(IXEmitter* e)   { if (e) { e->active = 0; e->spawnAccum = 0.0f; } }
void Emitter_Resume(IXEmitter* e) { if (e) { e->active = 1; e->paused = 0; } }
void Emitter_Pause(IXEmitter* e)  { if (e) e->paused = 1; }

void Emitter_Reset(IXEmitter* e)
{
    if (!e || !e->particles) return;
    std::memset(e->particles, 0, sizeof(IXParticle) * (size_t)e->capacity);
    e->activeCount = 0;
    e->spawnAccum  = 0.0f;
    e->active      = 1;
    e->paused      = 0;
}

// ---------------------------------------------------------------------------
//  Logica
// ---------------------------------------------------------------------------
void Emitter_Update(IXEmitter* e, float dt)
{
    if (!e || !e->particles || e->paused) return;

    // Simula particulas ativas
    for (int i = 0; i < e->capacity; i++)
    {
        IXParticle* p = &e->particles[i];
        if (!p->active) continue;

        p->life -= dt;
        if (p->life <= 0.0f)
        {
            p->active = 0;
            e->activeCount--;
            if (e->activeCount < 0) e->activeCount = 0;
            continue;
        }

        // Integracao de Euler
        p->vx += p->ax * dt;
        p->vy += p->ay * dt;
        p->x  += p->vx * dt;
        p->y  += p->vy * dt;
    }

    // Spawn continuo
    if (e->active && e->spawnRate > 0.0f)
    {
        e->spawnAccum += e->spawnRate * dt;
        while (e->spawnAccum >= 1.0f)
        {
            SpawnOne(e);
            e->spawnAccum -= 1.0f;
        }
    }
}

// ---------------------------------------------------------------------------
//  Rendering
// ---------------------------------------------------------------------------
void Emitter_Render(const IXEmitter* e)
{
    if (!e || !e->particles) return;

    IXCamera* cam = Camera_GetActive();

    for (int i = 0; i < e->capacity; i++)
    {
        const IXParticle* p = &e->particles[i];
        if (!p->active) continue;

        // Progresso da vida: 0 = recem nascida, 1 = prestes a morrer
        float t = (p->lifeMax > 0.0f)
            ? 1.0f - (p->life / p->lifeMax)
            : 1.0f;

        // Cor interpolada
        unsigned int color = LerpColor(p->colorStart, p->colorEnd, t);

        // Tamanho interpolado
        float size = (float)p->size + ((float)(p->sizeEnd - p->size)) * t;
        int   sz   = (int)size;
        if (sz < 1) sz = 1;

        // Posicao de tela
        int sx = (int)p->x;
        int sy = (int)p->y;
        int sw = sz;
        int sh = sz;

        if (cam)
        {
            Camera_WorldToScreen(cam, p->x, p->y, &sx, &sy);
            float zoom = cam->zoom > 0.0f ? cam->zoom : 1.0f;
            sw = (int)((float)sz * zoom);
            sh = sw;
            if (sw < 1) sw = sh = 1;
        }

        // Centraliza o quadrado na posicao
        sx -= sw / 2;
        sy -= sh / 2;

        if (e->texture)
        {
            Renderer_DrawSprite(e->texture,
                0, 0, e->texFrameW, e->texFrameH,
                sx, sy, sw, sh, color);
        }
        else
        {
            GUI_DrawFilledRect(sx, sy, sw, sh, color);
        }
    }
}

// ---------------------------------------------------------------------------
//  Queries
// ---------------------------------------------------------------------------
int Emitter_GetActiveCount(const IXEmitter* e) { return e ? e->activeCount : 0; }
int Emitter_IsEmpty(const IXEmitter* e)        { return e ? (e->activeCount == 0) : 1; }
