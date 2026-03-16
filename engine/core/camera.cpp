#include "camera.h"
#include "../renderer.h"
#include <cstring>
#include <cmath>

// ---------------------------------------------------------------------------
//  Camera global ativa  (usada por Entity_Render)
// ---------------------------------------------------------------------------
static IXCamera* g_activeCamera = NULL;

// ---------------------------------------------------------------------------
//  LCG simples para shake sem <cstdlib>
// ---------------------------------------------------------------------------
static unsigned int g_shakeRng = 0xCAFEBABE;
static float ShakeRand()   // retorna [-1, 1]
{
    g_shakeRng = g_shakeRng * 1664525u + 1013904223u;
    float t = (float)(int)(g_shakeRng & 0xFFFF) / (float)0x7FFF - 1.0f;
    return t;
}

// ---------------------------------------------------------------------------
//  Clamp interno
// ---------------------------------------------------------------------------
static float Clampf(float v, float lo, float hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

// ---------------------------------------------------------------------------
//  Aplica limites do mundo a posicao da camera
// ---------------------------------------------------------------------------
static void ApplyBounds(IXCamera* cam)
{
    if (!cam->boundsEnabled || cam->boundsW <= 0.0f || cam->boundsH <= 0.0f)
        return;

    float zoom  = cam->zoom > 0.0f ? cam->zoom : 1.0f;
    float vw    = (float)cam->viewportW / zoom;
    float vh    = (float)cam->viewportH / zoom;

    float minX  = cam->boundsX;
    float minY  = cam->boundsY;
    float maxX  = cam->boundsX + cam->boundsW - vw;
    float maxY  = cam->boundsY + cam->boundsH - vh;

    // Se o mundo e menor que o viewport, centraliza
    if (maxX < minX) { float mid = (minX + cam->boundsX + cam->boundsW) * 0.5f - vw * 0.5f; cam->x = mid; }
    else             cam->x = Clampf(cam->x, minX, maxX);

    if (maxY < minY) { float mid = (minY + cam->boundsY + cam->boundsH) * 0.5f - vh * 0.5f; cam->y = mid; }
    else             cam->y = Clampf(cam->y, minY, maxY);
}

// ---------------------------------------------------------------------------
//  Inicializacao
// ---------------------------------------------------------------------------
void Camera_Init(IXCamera* cam)
{
    if (!cam) return;
    std::memset(cam, 0, sizeof(IXCamera));
    cam->zoom       = 1.0f;
    cam->followLerp = 0.08f;
}

// ---------------------------------------------------------------------------
//  Configuracao
// ---------------------------------------------------------------------------
void Camera_SetPosition(IXCamera* cam, float x, float y)
{
    if (!cam) return;
    cam->x = x;
    cam->y = y;
    ApplyBounds(cam);
}

void Camera_SetZoom(IXCamera* cam, float zoom)
{
    if (!cam) return;
    if (zoom < 0.05f) zoom = 0.05f;
    cam->zoom = zoom;
    ApplyBounds(cam);
}

void Camera_SetWorldBounds(IXCamera* cam, float x, float y, float w, float h)
{
    if (!cam) return;
    if (w <= 0.0f || h <= 0.0f)
    {
        cam->boundsEnabled = 0;
        return;
    }
    cam->boundsEnabled = 1;
    cam->boundsX = x;
    cam->boundsY = y;
    cam->boundsW = w;
    cam->boundsH = h;
    ApplyBounds(cam);
}

void Camera_SetFollowLerp(IXCamera* cam, float lerp)
{
    if (!cam) return;
    cam->followLerp = Clampf(lerp, 0.0f, 1.0f);
}

// ---------------------------------------------------------------------------
//  Logica
// ---------------------------------------------------------------------------
void Camera_Follow(IXCamera* cam, float targetX, float targetY, float deltaSeconds)
{
    if (!cam) return;

    float zoom = cam->zoom > 0.0f ? cam->zoom : 1.0f;
    float vw   = (float)cam->viewportW / zoom;
    float vh   = (float)cam->viewportH / zoom;

    // Posicao da camera que centraliza o alvo
    float desiredX = targetX - vw * 0.5f;
    float desiredY = targetY - vh * 0.5f;

    // Lerp dependente do delta para ser frame-rate independent
    // formula: 1 - (1 - lerp)^(delta / (1/60))
    float t = 1.0f - (float)pow(1.0 - (double)cam->followLerp, (double)deltaSeconds * 60.0);
    t = Clampf(t, 0.0f, 1.0f);

    cam->x = cam->x + (desiredX - cam->x) * t;
    cam->y = cam->y + (desiredY - cam->y) * t;

    ApplyBounds(cam);
}

void Camera_CenterOn(IXCamera* cam, float targetX, float targetY)
{
    if (!cam) return;
    float zoom = cam->zoom > 0.0f ? cam->zoom : 1.0f;
    cam->x = targetX - (float)cam->viewportW / zoom * 0.5f;
    cam->y = targetY - (float)cam->viewportH / zoom * 0.5f;
    ApplyBounds(cam);
}

void Camera_Update(IXCamera* cam, float deltaSeconds)
{
    if (!cam) return;

    // Atualiza tamanho do viewport
    Renderer_GetClientSize(&cam->viewportW, &cam->viewportH);
    if (cam->viewportW < 1) cam->viewportW = 1;
    if (cam->viewportH < 1) cam->viewportH = 1;

    // Processa shake
    if (cam->shakeDuration > 0.0f)
    {
        cam->shakeDuration -= deltaSeconds;
        if (cam->shakeDuration <= 0.0f)
        {
            cam->shakeDuration = 0.0f;
            cam->shakeOffsetX  = 0.0f;
            cam->shakeOffsetY  = 0.0f;
        }
        else
        {
            // Intensidade diminui linearmente ate zero
            float ratio = cam->shakeDuration > 0.0f
                        ? Clampf(cam->shakeDuration / (cam->shakeDuration + deltaSeconds), 0.0f, 1.0f)
                        : 0.0f;
            float intensity = cam->shakeIntensity * ratio;
            cam->shakeOffsetX = ShakeRand() * intensity;
            cam->shakeOffsetY = ShakeRand() * intensity;
        }
    }
    else
    {
        cam->shakeOffsetX = 0.0f;
        cam->shakeOffsetY = 0.0f;
    }
}

void Camera_Shake(IXCamera* cam, float intensity, float duration)
{
    if (!cam || intensity <= 0.0f || duration <= 0.0f) return;
    // Acumula: pega o maior intensity e soma duracoes
    if (intensity > cam->shakeIntensity) cam->shakeIntensity = intensity;
    cam->shakeDuration += duration;
}

// ---------------------------------------------------------------------------
//  Transformacoes
// ---------------------------------------------------------------------------
void Camera_WorldToScreen(const IXCamera* cam,
                           float worldX, float worldY,
                           int* screenX, int* screenY)
{
    if (!cam) { if (screenX) *screenX = (int)worldX; if (screenY) *screenY = (int)worldY; return; }

    float zoom = cam->zoom > 0.0f ? cam->zoom : 1.0f;
    float sx   = (worldX - cam->x) * zoom + cam->shakeOffsetX;
    float sy   = (worldY - cam->y) * zoom + cam->shakeOffsetY;

    if (screenX) *screenX = (int)sx;
    if (screenY) *screenY = (int)sy;
}

void Camera_ScreenToWorld(const IXCamera* cam,
                           int screenX, int screenY,
                           float* worldX, float* worldY)
{
    if (!cam)
    {
        if (worldX) *worldX = (float)screenX;
        if (worldY) *worldY = (float)screenY;
        return;
    }
    float zoom = cam->zoom > 0.0f ? cam->zoom : 1.0f;
    if (worldX) *worldX = (float)(screenX - cam->shakeOffsetX) / zoom + cam->x;
    if (worldY) *worldY = (float)(screenY - cam->shakeOffsetY) / zoom + cam->y;
}

int Camera_IsVisible(const IXCamera* cam,
                     float worldX, float worldY, int w, int h)
{
    if (!cam) return 1;
    float zoom = cam->zoom > 0.0f ? cam->zoom : 1.0f;

    // Bounding box da entidade em coordenadas de tela
    float sx0 = (worldX       - cam->x) * zoom;
    float sy0 = (worldY       - cam->y) * zoom;
    float sx1 = (worldX + (float)w - cam->x) * zoom;
    float sy1 = (worldY + (float)h - cam->y) * zoom;

    if (sx1 < 0.0f || sx0 > (float)cam->viewportW) return 0;
    if (sy1 < 0.0f || sy0 > (float)cam->viewportH) return 0;
    return 1;
}

// ---------------------------------------------------------------------------
//  Attach / Detach
// ---------------------------------------------------------------------------
void Camera_Attach(IXCamera* cam) { g_activeCamera = cam; }
void Camera_Detach()              { g_activeCamera = NULL; }
IXCamera* Camera_GetActive()      { return g_activeCamera; }
