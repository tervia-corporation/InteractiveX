#include "renderer.h"
#include "gui.h"
#include "interface/button.h"
#include "interface/slider.h"
#include "interface/panel.h"
#include "core/transition.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <cstdio>

// ---------------------------------------------------------------------------
//  Vertex types
// ---------------------------------------------------------------------------
struct Vertex
{
    float x, y, z, rhw;
    float u, v;
};
#define IX_FVF (D3DFVF_XYZRHW | D3DFVF_TEX1)

struct SpriteVertex
{
    float        x, y, z, rhw;
    unsigned int color;
    float        u, v;
};
#define IX_SPRITE_FVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

// ---------------------------------------------------------------------------
//  Limites — 64 botoes e o suficiente para qualquer debug menu futuro
// ---------------------------------------------------------------------------
#define IX_MAX_OVERLAY_BUTTONS 64
#define IX_MAX_OVERLAY_SLIDERS 12

// ---------------------------------------------------------------------------
//  Device state
// ---------------------------------------------------------------------------
static LPDIRECT3D9           g_pD3D       = NULL;
static LPDIRECT3DDEVICE9     g_pd3dDevice = NULL;
static HWND                  g_hWnd       = NULL;
static D3DPRESENT_PARAMETERS g_d3dpp;

// ---------------------------------------------------------------------------
//  Managed textures
// ---------------------------------------------------------------------------
static LPDIRECT3DTEXTURE9 g_pixelBufTexture   = NULL;
static LPDIRECT3DTEXTURE9 g_backgroundTexture = NULL;
static LPDIRECT3DTEXTURE9 g_displayImgTexture = NULL;
static int g_pixelBufTexW    = 0;
static int g_pixelBufTexH    = 0;
static int g_displayImgWidth  = 0;
static int g_displayImgHeight = 0;

// ---------------------------------------------------------------------------
//  Frame guard
// ---------------------------------------------------------------------------
static int g_inFrame = 0;

// ---------------------------------------------------------------------------
//  Overlay panel data
// ---------------------------------------------------------------------------
static IXPanel     g_overlayPanel;
static const char* g_overlayTitle    = "InteractiveX";
static const char* g_overlaySubtitle = "";
static const char* g_overlayStatus   = "";
static const char* g_overlayHelp     = NULL;
static float       g_overlayRenderFPS = 0.0f;
static float       g_overlayFrameMs   = 0.0f;
static float       g_overlaySimUPS    = 0.0f;
static int         g_showFPS            = 1;
static int         g_showHelp           = 1;
static int         g_drawBackgroundGrid = 1;
static int         g_overlayEnabled     = 1;
static unsigned int g_backgroundColor  = 0xFF000000;

static const IXButton* g_controlButtons[IX_MAX_OVERLAY_BUTTONS];
static int             g_controlButtonCount = 0;
static const IXSlider* g_controlSliders[IX_MAX_OVERLAY_SLIDERS];
static int             g_controlSliderCount = 0;

// ---------------------------------------------------------------------------
//  Calcula a altura em pixels que as linhas de texto do overlay ocupam
// ---------------------------------------------------------------------------
static int Overlay_CalcTextHeight()
{
    // Subtitle e status sao sempre desenhados (mesmo que vazios)
    int h = 22 + 22;
    if (g_showFPS) h += 22;
    if (g_showHelp && g_overlayHelp && g_overlayHelp[0]) h += 22;
    return h;
}

// ---------------------------------------------------------------------------
//  Internal helpers
// ---------------------------------------------------------------------------
static void Renderer_ApplyRenderStates()
{
    g_pd3dDevice->SetRenderState(D3DRS_LIGHTING,         FALSE);
    g_pd3dDevice->SetRenderState(D3DRS_CULLMODE,         D3DCULL_NONE);
    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE,          FALSE);
    g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND,         D3DBLEND_SRCALPHA);
    g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND,        D3DBLEND_INVSRCALPHA);
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER,  D3DTEXF_LINEAR);
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER,  D3DTEXF_LINEAR);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
}

static void Renderer_RefreshOverlayPanelGeometry()
{
    int clientWidth  = 0;
    int clientHeight = 0;
    Renderer_GetClientSize(&clientWidth, &clientHeight);
    int panelHeight = clientHeight - 16;
    if (panelHeight < 200) panelHeight = 200;
    Panel_SetBounds(&g_overlayPanel, 8, 8, 360, panelHeight);
    Panel_SetTitle(&g_overlayPanel, g_overlayTitle);
}

static void Renderer_DrawTexturedQuad(IDirect3DTexture9* texture,
                                       float x, float y, float w, float h)
{
    if (!g_pd3dDevice || !texture) return;
    Vertex quad[4] = {
        { x,     y,     0.0f, 1.0f, 0.0f, 0.0f },
        { x + w, y,     0.0f, 1.0f, 1.0f, 0.0f },
        { x,     y + h, 0.0f, 1.0f, 0.0f, 1.0f },
        { x + w, y + h, 0.0f, 1.0f, 1.0f, 1.0f },
    };
    g_pd3dDevice->SetTexture(0, texture);
    g_pd3dDevice->SetFVF(IX_FVF);
    g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(Vertex));
}

static bool Renderer_EnsurePixelBufTexture(int width, int height)
{
    if (g_pixelBufTexture && g_pixelBufTexW == width && g_pixelBufTexH == height)
        return true;
    if (g_pixelBufTexture) { g_pixelBufTexture->Release(); g_pixelBufTexture = NULL; g_pixelBufTexW = g_pixelBufTexH = 0; }
    if (!g_pd3dDevice) return false;
    HRESULT hr = g_pd3dDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC,
        D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_pixelBufTexture, NULL);
    if (FAILED(hr)) return false;
    g_pixelBufTexW = width; g_pixelBufTexH = height;
    return true;
}

static void Renderer_ReleasePixelBufTexture()
{
    if (g_pixelBufTexture) { g_pixelBufTexture->Release(); g_pixelBufTexture = NULL; }
    g_pixelBufTexW = g_pixelBufTexH = 0;
}

static void Renderer_DrawBackdrop(int screenWidth, int screenHeight)
{
    if (g_backgroundTexture)
    {
        Renderer_DrawTexturedQuad(g_backgroundTexture, 0.0f, 0.0f, (float)screenWidth, (float)screenHeight);
        GUI_DrawFilledRect(0, 0, screenWidth, screenHeight, 0x18000000);
        return;
    }
    if (!g_drawBackgroundGrid) return;
    GUI_DrawFilledRect(0, 0, screenWidth, screenHeight, 0x08000000);
    for (int x = 0; x < screenWidth;  x += 32) GUI_DrawFilledRect(x, 0, 1, screenHeight, 0x1200A0FF);
    for (int y = 0; y < screenHeight; y += 32) GUI_DrawFilledRect(0, y, screenWidth, 1, 0x1200A0FF);
}

static void Renderer_DrawDisplayImage(int screenWidth, int screenHeight)
{
    if (!g_displayImgTexture || g_displayImgWidth <= 0 || g_displayImgHeight <= 0) return;
    const float availX = 392.0f;
    const float availY = 48.0f;
    float availW = (float)screenWidth  - availX - 24.0f; if (availW < 32.0f) availW = 32.0f;
    float availH = (float)screenHeight - availY - 24.0f; if (availH < 32.0f) availH = 32.0f;
    float drawW = (float)g_displayImgWidth, drawH = (float)g_displayImgHeight;
    if (drawW > availW || drawH > availH)
    {
        float scale = (availW / drawW < availH / drawH) ? availW / drawW : availH / drawH;
        drawW *= scale; drawH *= scale;
    }
    float drawX = availX + (availW - drawW) * 0.5f;
    float drawY = availY + (availH - drawH) * 0.5f;
    GUI_DrawFilledRect((int)availX - 8, (int)availY - 8, (int)availW + 16, (int)availH + 16, 0x22000000);
    GUI_DrawOutlinedRect((int)availX - 8, (int)availY - 8, (int)availW + 16, (int)availH + 16, 0x44FFFFFF);
    Renderer_DrawTexturedQuad(g_displayImgTexture, drawX, drawY, drawW, drawH);
}

static void Renderer_DrawOverlayPanel()
{
    char metricLine[128];
    Renderer_RefreshOverlayPanelGeometry();
    GUI_DrawPanel(&g_overlayPanel);

    int contentX  = g_overlayPanel.x + g_overlayPanel.padding;
    int textY     = g_overlayPanel.y + g_overlayPanel.headerHeight + 10;
    const int LINE = 22;

    // Linha 1 — subtitle
    GUI_DrawTextLine(contentX, textY, g_overlaySubtitle, 0xFFFFAA33); textY += LINE;

    // Linha 2 — status
    GUI_DrawTextLine(contentX, textY, g_overlayStatus, 0xFFD8D8D8); textY += LINE;

    // Linha 3 — metrics (opcional)
    if (g_showFPS)
    {
        std::snprintf(metricLine, sizeof(metricLine),
            "Render FPS: %.1f | Frame: %.2f ms | Sim UPS: %.1f",
            g_overlayRenderFPS, g_overlayFrameMs, g_overlaySimUPS);
        GUI_DrawTextLine(contentX, textY, metricLine, 0xFF9EE6A0);
        textY += LINE;
    }

    // Linha 4 — help (opcional)
    if (g_showHelp && g_overlayHelp && g_overlayHelp[0])
        GUI_DrawTextLine(contentX, textY, g_overlayHelp, 0xFFB8C7FF);

    // Botoes e sliders — posicionados por quem chamou SetControlButtons/Sliders
    for (int i = 0; i < g_controlButtonCount; i++) GUI_DrawButton(g_controlButtons[i]);
    for (int i = 0; i < g_controlSliderCount; i++) GUI_DrawSlider(g_controlSliders[i]);
}

// ---------------------------------------------------------------------------
//  Public API — core lifecycle
// ---------------------------------------------------------------------------
bool Renderer_Initialize(HWND hWnd)
{
    g_hWnd = hWnd;
    g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!g_pD3D) return false;
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed             = TRUE;
    g_d3dpp.SwapEffect           = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat     = D3DFMT_UNKNOWN;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    HRESULT hr = g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice);
    if (FAILED(hr)) return false;
    Renderer_ApplyRenderStates();
    GUI_Initialize();
    GUI_InternalCreate(g_pd3dDevice);
    Panel_Init(&g_overlayPanel, 8, 8, 360, 400, g_overlayTitle);
    return true;
}

void Renderer_HandleResize(int width, int height)
{
    if (!g_pd3dDevice || width <= 0 || height <= 0) return;
    Renderer_ReleasePixelBufTexture();
    GUI_Cleanup();
    g_d3dpp.BackBufferWidth  = width;
    g_d3dpp.BackBufferHeight = height;
    if (FAILED(g_pd3dDevice->Reset(&g_d3dpp))) return;
    Renderer_ApplyRenderStates();
    GUI_InternalCreate(g_pd3dDevice);
}

void Renderer_Cleanup()
{
    Renderer_ClearBackgroundImage();
    Renderer_ReleasePixelBufTexture();
    GUI_Cleanup();
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    if (g_pD3D)       { g_pD3D->Release();       g_pD3D       = NULL; }
}

// ---------------------------------------------------------------------------
//  Public API — queries
// ---------------------------------------------------------------------------
void Renderer_GetClientSize(int* width, int* height)
{
    RECT rc;
    if (!g_hWnd) { if (width) *width = 0; if (height) *height = 0; return; }
    GetClientRect(g_hWnd, &rc);
    if (width)  *width  = rc.right  - rc.left;
    if (height) *height = rc.bottom - rc.top;
}

void Renderer_GetOverlayContentRect(int* x, int* y, int* width, int* height)
{
    Renderer_RefreshOverlayPanelGeometry();
    if (x)      *x      = g_overlayPanel.x + g_overlayPanel.padding;
    if (y)      *y      = g_overlayPanel.y + g_overlayPanel.headerHeight + g_overlayPanel.padding;
    if (width)  *width  = g_overlayPanel.width  - (g_overlayPanel.padding * 2);
    if (height) *height = g_overlayPanel.height - g_overlayPanel.headerHeight - (g_overlayPanel.padding * 2);
    if (width  && *width  < 0) *width  = 0;
    if (height && *height < 0) *height = 0;
}

void Renderer_GetOverlayButtonsRect(int* x, int* y, int* width, int* height)
{
    Renderer_RefreshOverlayPanelGeometry();

    // Texto comeca logo apos o header + margem de 10px
    int textStartY = g_overlayPanel.y + g_overlayPanel.headerHeight + 10;

    // Altura total ocupada pelas linhas de texto
    int textH = Overlay_CalcTextHeight();

    // Botoes comecam 8px abaixo do ultimo texto
    int btnY = textStartY + textH + 8;

    // Base inferior do painel (com padding interno)
    int panelBottom = g_overlayPanel.y + g_overlayPanel.height - g_overlayPanel.padding;

    if (x)      *x      = g_overlayPanel.x + g_overlayPanel.padding;
    if (y)      *y      = btnY;
    if (width)  *width  = g_overlayPanel.width - g_overlayPanel.padding * 2;
    if (height) *height = panelBottom - btnY;
    if (height && *height < 0) *height = 0;
}

IDirect3DDevice9* Renderer_GetDevice() { return g_pd3dDevice; }

// ---------------------------------------------------------------------------
//  Public API — display image
// ---------------------------------------------------------------------------
void Renderer_SetDisplayImage(IDirect3DTexture9* texture, int width, int height)
{
    g_displayImgTexture = texture; g_displayImgWidth = width; g_displayImgHeight = height;
}
void Renderer_ClearDisplayImage()
{
    g_displayImgTexture = NULL; g_displayImgWidth = g_displayImgHeight = 0;
}

// ---------------------------------------------------------------------------
//  Public API — background image
// ---------------------------------------------------------------------------
bool Renderer_LoadBackgroundImage(const char* path)
{
    if (!g_pd3dDevice || !path || !path[0]) return false;
    Renderer_ClearBackgroundImage();
    HRESULT hr = D3DXCreateTextureFromFileExA(g_pd3dDevice, path,
        D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED,
        D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR, 0, NULL, NULL, &g_backgroundTexture);
    return SUCCEEDED(hr);
}
void Renderer_ClearBackgroundImage()
{
    if (g_backgroundTexture) { g_backgroundTexture->Release(); g_backgroundTexture = NULL; }
}

// ---------------------------------------------------------------------------
//  Public API — frame lifecycle
// ---------------------------------------------------------------------------
void Renderer_BeginFrame()
{
    if (!g_pd3dDevice) return;
    HRESULT coop = g_pd3dDevice->TestCooperativeLevel();
    if (coop == D3DERR_DEVICELOST) return;
    if (coop == D3DERR_DEVICENOTRESET) { int w, h; Renderer_GetClientSize(&w, &h); Renderer_HandleResize(w, h); return; }
    RECT rc; GetClientRect(g_hWnd, &rc);
    int screenW = rc.right - rc.left, screenH = rc.bottom - rc.top;
    g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, (D3DCOLOR)g_backgroundColor, 1.0f, 0);
    if (SUCCEEDED(g_pd3dDevice->BeginScene()))
    {
        GUI_BeginFrame();
        Renderer_DrawBackdrop(screenW, screenH);
        Renderer_DrawDisplayImage(screenW, screenH);
        g_inFrame = 1;
    }
}

void Renderer_DrawPixelBuffer(const unsigned int* pixels,
                               int srcWidth, int srcHeight,
                               int dstX, int dstY, int dstWidth, int dstHeight)
{
    if (!g_inFrame || !pixels || srcWidth <= 0 || srcHeight <= 0 || dstWidth <= 0 || dstHeight <= 0) return;
    if (!Renderer_EnsurePixelBufTexture(srcWidth, srcHeight)) return;
    D3DLOCKED_RECT locked;
    if (SUCCEEDED(g_pixelBufTexture->LockRect(0, &locked, NULL, D3DLOCK_DISCARD)))
    {
        for (int y = 0; y < srcHeight; y++)
        {
            unsigned int* dst = (unsigned int*)((unsigned char*)locked.pBits + y * locked.Pitch);
            const unsigned int* src = pixels + y * srcWidth;
            for (int x = 0; x < srcWidth; x++) dst[x] = src[x];
        }
        g_pixelBufTexture->UnlockRect(0);
    }
    Renderer_DrawTexturedQuad(g_pixelBufTexture, (float)dstX, (float)dstY, (float)dstWidth, (float)dstHeight);
}

void Renderer_DrawSprite(IDirect3DTexture9* texture,
                          int srcX, int srcY, int srcW, int srcH,
                          int dstX, int dstY, int dstW, int dstH,
                          unsigned int tint)
{
    if (!g_inFrame || !g_pd3dDevice || !texture || dstW <= 0 || dstH <= 0) return;
    D3DSURFACE_DESC desc;
    if (FAILED(texture->GetLevelDesc(0, &desc))) return;
    float texW = (float)desc.Width, texH = (float)desc.Height;
    if (srcW <= 0) srcW = (int)texW;
    if (srcH <= 0) srcH = (int)texH;
    float u0 = srcX / texW, v0 = srcY / texH;
    float u1 = (srcX + srcW) / texW, v1 = (srcY + srcH) / texH;
    float x0 = (float)dstX, y0 = (float)dstY, x1 = (float)(dstX + dstW), y1 = (float)(dstY + dstH);
    SpriteVertex quad[4] = {
        { x0, y0, 0.0f, 1.0f, tint, u0, v0 },
        { x1, y0, 0.0f, 1.0f, tint, u1, v0 },
        { x0, y1, 0.0f, 1.0f, tint, u0, v1 },
        { x1, y1, 0.0f, 1.0f, tint, u1, v1 },
    };
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    g_pd3dDevice->SetTexture(0, texture);
    g_pd3dDevice->SetFVF(IX_SPRITE_FVF);
    g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(SpriteVertex));
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_pd3dDevice->SetFVF(IX_FVF);
}

void Renderer_EndFrame()
{
    if (!g_inFrame) return;
    g_inFrame = 0;
    RECT rc; GetClientRect(g_hWnd, &rc);
    int screenW = rc.right - rc.left;
    int screenH = rc.bottom - rc.top;
    if (g_overlayEnabled) Renderer_DrawOverlayPanel();
    Transition_DrawOverlay(screenW, screenH);
    GUI_EndFrame();
    g_pd3dDevice->EndScene();
    g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

// ---------------------------------------------------------------------------
//  Public API — overlay configuration
// ---------------------------------------------------------------------------
void Renderer_SetOverlayText(const char* title, const char* subtitle, const char* statusLine)
{
    g_overlayTitle = title; g_overlaySubtitle = subtitle; g_overlayStatus = statusLine;
}
void Renderer_SetOverlayMetrics(float renderFps, float frameMs, float simUps)
{
    g_overlayRenderFPS = renderFps; g_overlayFrameMs = frameMs; g_overlaySimUPS = simUps;
}
void Renderer_SetControlButton(const IXButton* button)
{
    g_controlButtonCount = 0;
    if (button) { g_controlButtons[0] = button; g_controlButtonCount = 1; }
}
void Renderer_SetControlButtons(const IXButton** buttons, int count)
{
    g_controlButtonCount = 0;
    if (!buttons || count <= 0) return;
    if (count > IX_MAX_OVERLAY_BUTTONS) count = IX_MAX_OVERLAY_BUTTONS;
    for (int i = 0; i < count; i++) g_controlButtons[i] = buttons[i];
    g_controlButtonCount = count;
}
void Renderer_SetControlSliders(const IXSlider** sliders, int count)
{
    g_controlSliderCount = 0;
    if (!sliders || count <= 0) return;
    if (count > IX_MAX_OVERLAY_SLIDERS) count = IX_MAX_OVERLAY_SLIDERS;
    for (int i = 0; i < count; i++) g_controlSliders[i] = sliders[i];
    g_controlSliderCount = count;
}
void Renderer_SetDisplayOptions(int showFPS, int showHelp, int drawBackgroundGrid)
{
    g_showFPS = showFPS; g_showHelp = showHelp; g_drawBackgroundGrid = drawBackgroundGrid;
}
void Renderer_SetBackgroundColor(unsigned int argb) { g_backgroundColor = argb; }
void Renderer_SetHelpText(const char* helpText)     { g_overlayHelp = helpText; }
void Renderer_SetOverlayEnabled(int enabled)        { g_overlayEnabled = enabled ? 1 : 0; }

void Renderer_SetVSync(int vsync)
{
    UINT interval = vsync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
    if (g_d3dpp.PresentationInterval != interval)
        g_d3dpp.PresentationInterval = interval;
    // O device sera resetado pelo proximo Renderer_HandleResize chamado pelo Video_Apply
}
