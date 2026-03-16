#include "animation.h"
#include "entity.h"
#include "../renderer.h"
#include <cstring>
#include <cstdio>
#include <d3d9.h>

// ---------------------------------------------------------------------------
//  Helpers internos
// ---------------------------------------------------------------------------
static int FindClip(const IXAnimation* anim, const char* name)
{
    if (!anim || !name) return -1;
    for (int i = 0; i < anim->clipCount; i++)
        if (std::strncmp(anim->clips[i].name, name, IX_ANIM_NAME_LEN) == 0)
            return i;
    return -1;
}

// Calcula col/row de um frame e preenche srcX/Y
static void FrameToUV(const IXAnimation* anim, int frameIndex,
                       int* srcX, int* srcY)
{
    int cols = anim->cols > 0 ? anim->cols : 1;
    int col  = frameIndex % cols;
    int row  = frameIndex / cols;
    if (srcX) *srcX = col * anim->frameW;
    if (srcY) *srcY = row * anim->frameH;
}

// Consulta dimensoes reais da textura via D3D9
static void QueryTextureDimensions(IXAnimation* anim)
{
    anim->texW = 0;
    anim->texH = 0;
    anim->cols = 1;
    if (!anim->texture) return;

    D3DSURFACE_DESC desc;
    if (SUCCEEDED(anim->texture->GetLevelDesc(0, &desc)))
    {
        anim->texW = (int)desc.Width;
        anim->texH = (int)desc.Height;
        anim->cols = (anim->frameW > 0) ? (anim->texW / anim->frameW) : 1;
        if (anim->cols < 1) anim->cols = 1;
    }
}

// ---------------------------------------------------------------------------
//  Inicializacao
// ---------------------------------------------------------------------------
void Animation_Init(IXAnimation* anim,
                    IDirect3DTexture9* texture,
                    int frameW, int frameH)
{
    if (!anim) return;
    std::memset(anim, 0, sizeof(IXAnimation));
    anim->texture      = texture;
    anim->frameW       = frameW > 0 ? frameW : 1;
    anim->frameH       = frameH > 0 ? frameH : 1;
    anim->currentClip  = -1;
    anim->currentFrame = 0;
    anim->timer        = 0.0f;
    anim->finished     = 0;
    anim->paused       = 0;
    anim->clipCount    = 0;
    anim->onClipEnd    = NULL;
    QueryTextureDimensions(anim);
}

void Animation_SetTexture(IXAnimation* anim, IDirect3DTexture9* texture)
{
    if (!anim) return;
    anim->texture = texture;
    QueryTextureDimensions(anim);
}

// ---------------------------------------------------------------------------
//  Registro de clips
// ---------------------------------------------------------------------------
int Animation_AddClip(IXAnimation* anim,
                      const char* name,
                      int firstFrame, int frameCount,
                      float fps, int loop)
{
    if (!anim || !name) return 0;

    // Substitui se ja existe
    int idx = FindClip(anim, name);
    if (idx < 0)
    {
        if (anim->clipCount >= IX_ANIM_MAX_CLIPS) return 0;
        idx = anim->clipCount++;
    }

    IXAnimClip* c = &anim->clips[idx];
    std::strncpy(c->name, name, IX_ANIM_NAME_LEN - 1);
    c->name[IX_ANIM_NAME_LEN - 1] = '\0';
    c->firstFrame  = firstFrame;
    c->frameCount  = frameCount > 0 ? frameCount : 1;
    c->fps         = fps > 0.0f    ? fps         : 1.0f;
    c->loop        = loop ? 1 : 0;
    return 1;
}

// ---------------------------------------------------------------------------
//  Controle de reproducao
// ---------------------------------------------------------------------------
int Animation_Play(IXAnimation* anim, const char* clipName)
{
    if (!anim || !clipName) return 0;
    int idx = FindClip(anim, clipName);
    if (idx < 0) return 0;

    anim->currentClip  = idx;
    anim->currentFrame = 0;
    anim->timer        = 0.0f;
    anim->finished     = 0;
    anim->paused       = 0;
    return 1;
}

void Animation_Restart(IXAnimation* anim)
{
    if (!anim || anim->currentClip < 0) return;
    anim->currentFrame = 0;
    anim->timer        = 0.0f;
    anim->finished     = 0;
    anim->paused       = 0;
}

void Animation_Pause(IXAnimation* anim)
{
    if (!anim) return;
    anim->paused = 1;
}

void Animation_Resume(IXAnimation* anim)
{
    if (!anim) return;
    anim->paused = 0;
}

void Animation_Stop(IXAnimation* anim)
{
    if (!anim) return;
    anim->currentFrame = 0;
    anim->timer        = 0.0f;
    anim->finished     = 0;
    anim->paused       = 0;
}

void Animation_SetOnClipEnd(IXAnimation* anim, IXAnimationCallback cb)
{
    if (!anim) return;
    anim->onClipEnd = cb;
}

// ---------------------------------------------------------------------------
//  Queries
// ---------------------------------------------------------------------------
const char* Animation_GetCurrentClipName(const IXAnimation* anim)
{
    if (!anim || anim->currentClip < 0) return "";
    return anim->clips[anim->currentClip].name;
}

int Animation_GetCurrentFrame(const IXAnimation* anim)
{
    return anim ? anim->currentFrame : 0;
}

int Animation_IsFinished(const IXAnimation* anim)
{
    return anim ? anim->finished : 0;
}

int Animation_IsPaused(const IXAnimation* anim)
{
    return anim ? anim->paused : 0;
}

// ---------------------------------------------------------------------------
//  Logica
// ---------------------------------------------------------------------------
void Animation_Update(IXAnimation* anim, float deltaSeconds)
{
    if (!anim || anim->currentClip < 0) return;
    if (anim->paused || anim->finished)  return;

    const IXAnimClip* clip = &anim->clips[anim->currentClip];

    anim->timer += deltaSeconds;

    float frameDuration = (clip->fps > 0.0f) ? (1.0f / clip->fps) : 1.0f;

    while (anim->timer >= frameDuration)
    {
        anim->timer -= frameDuration;
        anim->currentFrame++;

        if (anim->currentFrame >= clip->frameCount)
        {
            if (clip->loop)
            {
                anim->currentFrame = 0;
            }
            else
            {
                anim->currentFrame = clip->frameCount - 1;
                anim->finished     = 1;
                if (anim->onClipEnd)
                    anim->onClipEnd(anim, clip->name);
                return;
            }
        }
    }
}

// ---------------------------------------------------------------------------
//  Rendering
// ---------------------------------------------------------------------------
void Animation_Render(const IXAnimation* anim,
                       int dstX, int dstY, int dstW, int dstH,
                       unsigned int tint)
{
    if (!anim || !anim->texture) return;

    int absFrame = (anim->currentClip >= 0)
        ? anim->clips[anim->currentClip].firstFrame + anim->currentFrame
        : 0;

    int srcX, srcY;
    FrameToUV(anim, absFrame, &srcX, &srcY);

    Renderer_DrawSprite(anim->texture,
        srcX, srcY, anim->frameW, anim->frameH,
        dstX, dstY, dstW, dstH,
        tint);
}

void Animation_ApplyToEntity(const IXAnimation* anim, IXEntity* entity)
{
    if (!anim || !entity) return;

    int absFrame = (anim->currentClip >= 0)
        ? anim->clips[anim->currentClip].firstFrame + anim->currentFrame
        : 0;

    int srcX, srcY;
    FrameToUV(anim, absFrame, &srcX, &srcY);

    entity->texture = anim->texture;
    entity->srcX    = srcX;
    entity->srcY    = srcY;
    entity->srcW    = anim->frameW;
    entity->srcH    = anim->frameH;
}
