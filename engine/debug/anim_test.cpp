#include "anim_test.h"
#include "../core/animation.h"
#include "../core/entity.h"
#include "../core/camera.h"
#include "../core/input.h"
#include "../core/timer.h"
#include "../renderer.h"
#include "../gui.h"
#include <cstdio>
#include <cstring>
#include <d3d9.h>

// ---------------------------------------------------------------------------
//  Configuracao da spritesheet sintetica
//
//  Gera uma textura de 256x64 com 8 frames de 64x64, cada um colorido
//  e com o numero do frame pintado como padrão de pixels — sem precisar
//  de arquivo externo. Demonstra o sistema funcionando do zero.
// ---------------------------------------------------------------------------
#define FRAME_W     64
#define FRAME_H     64
#define FRAME_COUNT  8
#define SHEET_W    (FRAME_W * FRAME_COUNT)
#define SHEET_H     FRAME_H

// Cores dos frames (ARGB)
static const unsigned int FRAME_COLORS[FRAME_COUNT] = {
    0xFFE05050, 0xFFE08030, 0xFFD0C030, 0xFF50C040,
    0xFF3090D0, 0xFF6050D0, 0xFFB040B0, 0xFFD04080,
};

// ---------------------------------------------------------------------------
//  Estado
// ---------------------------------------------------------------------------
static IDirect3DTexture9* g_sheetTex   = NULL;
static IXAnimation        g_anim;
static IXEntity           g_entity;
static IXCamera           g_cam;

static int  g_returnToMenu = 0;
static char g_statusLine[256] = "Animation test ready.";
static char g_helpLine[256]   = "1-3 clips | +/- fps | SPACE pause | R restart | ESC menu";

static float g_currentFPS = 8.0f;

// Nomes dos clips de demonstracao
static const char* CLIP_NAMES[]  = { "idle", "run", "attack" };
static const int   CLIP_STARTS[] = { 0,       2,     5       };
static const int   CLIP_COUNTS[] = { 2,       3,     3       };
static const int   CLIP_LOOPS[]  = { 1,       1,     0       };  // attack = once

// ---------------------------------------------------------------------------
//  Gera textura sintetica com frames coloridos
// ---------------------------------------------------------------------------
static void BuildSpritesheet()
{
    IDirect3DDevice9* dev = Renderer_GetDevice();
    if (!dev) return;

    // Libera anterior
    if (g_sheetTex) { g_sheetTex->Release(); g_sheetTex = NULL; }

    HRESULT hr = dev->CreateTexture(
        SHEET_W, SHEET_H, 1, D3DUSAGE_DYNAMIC,
        D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
        &g_sheetTex, NULL);
    if (FAILED(hr)) return;

    D3DLOCKED_RECT locked;
    if (FAILED(g_sheetTex->LockRect(0, &locked, NULL, D3DLOCK_DISCARD))) return;

    for (int f = 0; f < FRAME_COUNT; f++)
    {
        unsigned int base = FRAME_COLORS[f];
        unsigned int dark = ((base & 0xFEFEFEFE) >> 1) | 0xFF000000;

        for (int py = 0; py < FRAME_H; py++)
        {
            unsigned int* row = (unsigned int*)((unsigned char*)locked.pBits
                + py * locked.Pitch)
                + f * FRAME_W;

            for (int px = 0; px < FRAME_W; px++)
            {
                // Borda de 3px escura
                int onBorder = (px < 3 || px >= FRAME_W - 3 ||
                                py < 3 || py >= FRAME_H - 3);
                // Cruz central (mostra o frame number em binario)
                int bit      = (f >> ((px - FRAME_W/2 + 3) / 6)) & 1;
                int onCross  = (px >= FRAME_W/2 - 3 && px < FRAME_W/2 + 3) ||
                               (py >= FRAME_H/2 - 3 && py < FRAME_H/2 + 3);
                // Brilho no canto superior-esquerdo
                int shine    = (px + py < 20) ? 1 : 0;

                unsigned int color;
                if (onBorder)
                    color = 0xFF000000;
                else if (onCross && bit)
                    color = 0xFFFFFFFF;
                else if (shine)
                    color = base | 0xFF808080;  // clareia
                else
                    color = base;

                // Pulsa: frames pares um pouco mais escuros
                if (!onBorder && !shine && (f & 1) == 0)
                    color = ((color & 0xFEFEFEFE) >> 1) | (dark & 0xFF000000);

                row[px] = color;
            }
        }
    }
    g_sheetTex->UnlockRect(0);
}

// ---------------------------------------------------------------------------
//  Callback: clip "attack" terminou
// ---------------------------------------------------------------------------
static void OnClipEnd(IXAnimation* anim, const char* clipName)
{
    (void)anim; (void)clipName;
    // Volta automaticamente pra "idle" quando attack termina
    Animation_Play(&g_anim, "idle");
}

// ---------------------------------------------------------------------------
//  Public API
// ---------------------------------------------------------------------------
void DebugAnimTest_Initialize()
{
    g_returnToMenu = 0;
    g_currentFPS   = 8.0f;

    // Gera spritesheet sintetica
    BuildSpritesheet();

    // Inicializa animacao
    Animation_Init(&g_anim, g_sheetTex, FRAME_W, FRAME_H);

    for (int i = 0; i < 3; i++)
        Animation_AddClip(&g_anim, CLIP_NAMES[i],
            CLIP_STARTS[i], CLIP_COUNTS[i],
            g_currentFPS, CLIP_LOOPS[i]);

    Animation_SetOnClipEnd(&g_anim, OnClipEnd);
    Animation_Play(&g_anim, "idle");

    // Entidade para mostrar Animation_ApplyToEntity
    Entity_Init(&g_entity, 500.0f, 200.0f, FRAME_W * 2, FRAME_H * 2);

    // Camera simples (sem follow, so para usar DrawSprite via attach)
    Camera_Init(&g_cam);
    Camera_Update(&g_cam, 0.0f);
}

void DebugAnimTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased)
{
    (void)mouseX; (void)mouseY; (void)mouseDown; (void)mouseReleased;

    if (Input_WasKeyPressed(VK_ESCAPE)) { g_returnToMenu = 1; return; }

    // Troca clip
    if (Input_WasKeyPressed('1')) Animation_Play(&g_anim, "idle");
    if (Input_WasKeyPressed('2')) Animation_Play(&g_anim, "run");
    if (Input_WasKeyPressed('3')) Animation_Play(&g_anim, "attack");

    // FPS
    if (Input_WasKeyPressed(VK_OEM_PLUS)  || Input_WasKeyPressed(VK_ADD))
    {
        g_currentFPS += 2.0f;
        if (g_currentFPS > 60.0f) g_currentFPS = 60.0f;
        // Re-registra clips com novo fps
        for (int i = 0; i < 3; i++)
            Animation_AddClip(&g_anim, CLIP_NAMES[i],
                CLIP_STARTS[i], CLIP_COUNTS[i],
                g_currentFPS, CLIP_LOOPS[i]);
    }
    if (Input_WasKeyPressed(VK_OEM_MINUS) || Input_WasKeyPressed(VK_SUBTRACT))
    {
        g_currentFPS -= 2.0f;
        if (g_currentFPS < 1.0f) g_currentFPS = 1.0f;
        for (int i = 0; i < 3; i++)
            Animation_AddClip(&g_anim, CLIP_NAMES[i],
                CLIP_STARTS[i], CLIP_COUNTS[i],
                g_currentFPS, CLIP_LOOPS[i]);
    }

    // Pause / resume
    if (Input_WasKeyPressed(VK_SPACE))
    {
        if (Animation_IsPaused(&g_anim)) Animation_Resume(&g_anim);
        else                              Animation_Pause(&g_anim);
    }

    // Restart
    if (Input_WasKeyPressed('R')) Animation_Restart(&g_anim);

    // Update
    float dt = Timer_GetDeltaSeconds();
    Animation_Update(&g_anim, dt);

    // Aplica frame atual na entity tambem
    Animation_ApplyToEntity(&g_anim, &g_entity);

    // Status
    std::snprintf(g_statusLine, sizeof(g_statusLine),
        "Clip: %s | Frame: %d | FPS: %.0f | %s%s",
        Animation_GetCurrentClipName(&g_anim),
        Animation_GetCurrentFrame(&g_anim),
        g_currentFPS,
        Animation_IsPaused(&g_anim)  ? "pausado"  : "tocando",
        Animation_IsFinished(&g_anim) ? " | fim"   : "");
}

void DebugAnimTest_Render()
{
    int cw = 0, ch = 0;
    Renderer_GetClientSize(&cw, &ch);

    // -----------------------------------------------------------------------
    //  1. Spritesheet completa no topo (sem camera)
    // -----------------------------------------------------------------------
    int sheetDstX = 380;
    int sheetDstY = 50;
    int sheetDstW = SHEET_W;   // 1:1
    int sheetDstH = SHEET_H;

    GUI_DrawFilledRect(sheetDstX - 4, sheetDstY - 4,
        sheetDstW + 8, sheetDstH + 8, 0x22000000);
    GUI_DrawOutlinedRect(sheetDstX - 4, sheetDstY - 4,
        sheetDstW + 8, sheetDstH + 8, 0x33FFFFFF);

    Renderer_DrawSprite(g_sheetTex, 0, 0, 0, 0,
        sheetDstX, sheetDstY, sheetDstW, sheetDstH, 0xFFFFFFFF);

    // Destaca o frame atual na spritesheet
    if (g_anim.currentClip >= 0)
    {
        int absFrame = g_anim.clips[g_anim.currentClip].firstFrame
                     + g_anim.currentFrame;
        int hx = sheetDstX + absFrame * FRAME_W - 2;
        int hy = sheetDstY - 2;
        GUI_DrawOutlinedRect(hx, hy, FRAME_W + 4, FRAME_H + 4, 0xFFFFFF44);
    }

    GUI_DrawTextLine(sheetDstX, sheetDstY + FRAME_H + 8,
        "spritesheet (8 frames de 64x64)", 0xFF909090);

    // -----------------------------------------------------------------------
    //  2. Frame atual ampliado (3x) — Animation_Render direto
    // -----------------------------------------------------------------------
    int bigX = 380;
    int bigY = 180;
    int bigW = FRAME_W * 3;
    int bigH = FRAME_H * 3;

    GUI_DrawFilledRect(bigX - 4, bigY - 4, bigW + 8, bigH + 8, 0x22000000);
    GUI_DrawOutlinedRect(bigX - 4, bigY - 4, bigW + 8, bigH + 8, 0x44FFFFFF);
    Animation_Render(&g_anim, bigX, bigY, bigW, bigH, 0xFFFFFFFF);
    GUI_DrawTextLine(bigX, bigY + bigH + 8, "Animation_Render (3x)", 0xFF909090);

    // -----------------------------------------------------------------------
    //  3. Entity com Animation_ApplyToEntity (2x, com camera)
    // -----------------------------------------------------------------------
    Camera_Attach(&g_cam);
    Entity_Render(&g_entity);
    Camera_Detach();

    GUI_DrawTextLine((int)g_entity.x, (int)g_entity.y + g_entity.h + 8,
        "ApplyToEntity (2x)", 0xFF909090);

    // -----------------------------------------------------------------------
    //  4. Barra de progresso do clip
    // -----------------------------------------------------------------------
    if (g_anim.currentClip >= 0)
    {
        const IXAnimClip* clip = &g_anim.clips[g_anim.currentClip];
        int barX = 380;
        int barY = ch - 40;
        int barW = cw - barX - 20;
        int barH = 10;

        float progress = (clip->frameCount > 1)
            ? (float)g_anim.currentFrame / (float)(clip->frameCount - 1)
            : 1.0f;

        GUI_DrawFilledRect(barX, barY, barW, barH, 0x33FFFFFF);
        GUI_DrawFilledRect(barX, barY, (int)(barW * progress), barH, 0xFFFFAA33);
        GUI_DrawOutlinedRect(barX, barY, barW, barH, 0x55FFFFFF);

        char barLabel[64];
        std::snprintf(barLabel, sizeof(barLabel), "%s — frame %d / %d",
            clip->name, g_anim.currentFrame + 1, clip->frameCount);
        GUI_DrawTextLine(barX, barY - 20, barLabel, 0xFFD0D0D0);
    }
}

void DebugAnimTest_ApplyRendererState(float renderFPS, float frameMs)
{
    Renderer_SetOverlayText("InteractiveX r0.2", "Animation test", g_statusLine);
    Renderer_SetHelpText(g_helpLine);
    Renderer_SetDisplayOptions(1, 1, 0);
    Renderer_SetControlButton(NULL);
    Renderer_SetControlButtons(NULL, 0);
    Renderer_SetControlSliders(NULL, 0);
    Renderer_SetOverlayMetrics(renderFPS, frameMs, 0.0f);
}

void DebugAnimTest_HandleKeyDown(unsigned int key)
{
    if (key == VK_ESCAPE) g_returnToMenu = 1;
}

int  DebugAnimTest_ShouldReturnToMenu() { return g_returnToMenu; }
void DebugAnimTest_ClearReturnToMenu()
{
    if (g_sheetTex) { g_sheetTex->Release(); g_sheetTex = NULL; }
    g_returnToMenu = 0;
}
