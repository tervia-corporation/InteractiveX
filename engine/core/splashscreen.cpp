#include "splashscreen.h"
#include "math2d.h"
#include "../renderer.h"
#include "../gui.h"
#include <d3d9.h>
#include <d3dx9.h>

// ---------------------------------------------------------------------------
//  Helpers
// ---------------------------------------------------------------------------
static void Splash_ProcessMessages(HWND hWnd, int* outQuit, int* outSkip)
{
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            *outQuit = 1;
            PostQuitMessage(0);
            return;
        }
        if (msg.message == WM_KEYDOWN)
        {
            WPARAM k = msg.wParam;
            if (k == VK_ESCAPE || k == VK_RETURN || k == VK_SPACE)
                *outSkip = 1;
        }
        if (msg.message == WM_LBUTTONDOWN)
            *outSkip = 1;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

static void CalcCoverRect(int imgW, int imgH, int scrW, int scrH,
                           int* outX, int* outY, int* outW, int* outH)
{
    float scaleX = (float)scrW / (float)imgW;
    float scaleY = (float)scrH / (float)imgH;
    float scale  = scaleX > scaleY ? scaleX : scaleY;
    int dw = (int)((float)imgW * scale);
    int dh = (int)((float)imgH * scale);
    *outX = (scrW - dw) / 2;
    *outY = (scrH - dh) / 2;
    *outW = dw;
    *outH = dh;
}

// ---------------------------------------------------------------------------
//  Implementacao
// ---------------------------------------------------------------------------
void Splashscreen_Show(HWND hWnd, const char* imagePath, float durationSeconds)
{
    Splashscreen_ShowEx(hWnd, imagePath, durationSeconds, 0.5f, 0.5f, 0xFF000000, 0);
}

void Splashscreen_ShowEx(HWND hWnd,
                          const char* imagePath,
                          float durationSeconds,
                          float fadeInSeconds,
                          float fadeOutSeconds,
                          unsigned int bgColor,
                          int skipFadeOnExit)
{
    if (!hWnd) return;

    Renderer_SetOverlayEnabled(0);

    const char* path = (imagePath && imagePath[0]) ? imagePath : "assets/splash/bg.png";

    // Carrega textura via D3DX (sem dependencia de image.cpp/GDI+)
    IDirect3DDevice9*  device  = Renderer_GetDevice();
    IDirect3DTexture9* texture = NULL;
    int texW = 0, texH = 0;

    if (device)
    {
        D3DXIMAGE_INFO info;
        HRESULT hr = D3DXCreateTextureFromFileExA(
            device, path,
            D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2,
            1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED,
            D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR,
            0, &info, NULL, &texture);

        if (SUCCEEDED(hr) && texture)
        {
            texW = (int)info.Width;
            texH = (int)info.Height;
        }
    }

    // Normaliza duracao
    if (durationSeconds < 0.1f) durationSeconds = 0.1f;
    if (fadeInSeconds   < 0.0f) fadeInSeconds   = 0.0f;
    if (fadeOutSeconds  < 0.0f) fadeOutSeconds  = 0.0f;
    if (fadeInSeconds + fadeOutSeconds > durationSeconds)
    {
        float total = fadeInSeconds + fadeOutSeconds;
        fadeInSeconds  = fadeInSeconds  / total * durationSeconds;
        fadeOutSeconds = fadeOutSeconds / total * durationSeconds;
    }

    LARGE_INTEGER freq, startTime, now;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&startTime);

    int   quit = 0, skip = 0, exitFade = 0;
    float exitFadeElapsed  = 0.0f;
    float exitFadeDuration = skipFadeOnExit ? 0.0f : fadeOutSeconds;
    float prevTime = 0.0f;

    while (!quit)
    {
        QueryPerformanceCounter(&now);
        float elapsed = (float)(now.QuadPart - startTime.QuadPart) / (float)freq.QuadPart;
        float dt = elapsed - prevTime;
        prevTime = elapsed;

        Splash_ProcessMessages(hWnd, &quit, &skip);
        if (quit) break;

        if (skip && !exitFade)
        {
            exitFade = 1;
            exitFadeElapsed = 0.0f;
            if (skipFadeOnExit) break;
        }

        if (!exitFade && elapsed >= durationSeconds - fadeOutSeconds)
            exitFade = 1;

        if (exitFade)
        {
            exitFadeElapsed += dt;
            if (exitFadeElapsed >= exitFadeDuration) break;
        }

        // Calcula alpha
        float alpha = 1.0f;
        if (exitFade)
        {
            float t = (exitFadeDuration > 0.0f)
                      ? IX_Clamp01(exitFadeElapsed / exitFadeDuration)
                      : 1.0f;
            alpha = 1.0f - t;
        }
        else if (elapsed < fadeInSeconds && fadeInSeconds > 0.0f)
        {
            alpha = IX_Clamp01(elapsed / fadeInSeconds);
        }

        // Render
        Renderer_BeginFrame();

        int scrW = 0, scrH = 0;
        Renderer_GetClientSize(&scrW, &scrH);
        GUI_DrawFilledRect(0, 0, scrW, scrH, bgColor);

        if (texture && texW > 0 && texH > 0)
        {
            int dx, dy, dw, dh;
            CalcCoverRect(texW, texH, scrW, scrH, &dx, &dy, &dw, &dh);
            unsigned int a    = (unsigned int)(alpha * 255.0f);
            if (a > 255) a = 255;
            unsigned int tint = (a << 24) | 0x00FFFFFF;
            GUI_DrawSprite(texture, 0, 0, texW, texH, dx, dy, dw, dh, tint);
        }

        Renderer_EndFrame();
        Sleep(1);
    }

    if (texture) texture->Release();
    Renderer_SetOverlayEnabled(1);
}
