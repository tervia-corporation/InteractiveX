// ---------------------------------------------------------------------------
//  pl_mpeg — header-only MPEG-1 decoder
//  Coloque pl_mpeg.h em: engine/third_party/pl_mpeg/pl_mpeg.h
//  Download: https://github.com/phoboslab/pl_mpeg (arquivo unico)
// ---------------------------------------------------------------------------
#include <stdio.h>      // pl_mpeg requer FILE antes de ser incluido
#define PL_MPEG_IMPLEMENTATION
#include "../third_party/pl_mpeg/pl_mpeg.h"

#include "video_player.h"
#include "../renderer.h"
#include "../gui.h"
#include "../../engine/core/math2d.h"

// miniaudio ja esta presente na engine — inclui sem reimplementar
#define MINIAUDIO_IMPLEMENTATION_ALREADY_DONE
#include "../third_party/miniaudio/miniaudio.h"

#include <d3d9.h>
#include <cstdio>
#include <cstring>
#include <cmath>

// ---------------------------------------------------------------------------
//  Ring buffer de PCM para sincronizar audio do video
//  (thread-safe por indices atomicos — audio callback roda em outra thread)
// ---------------------------------------------------------------------------
#define IX_VID_RING_SIZE  (48000 * 2 * 4)  // ~1s de PCM float32 estereo 48kHz

static float     g_ringBuf[IX_VID_RING_SIZE / sizeof(float)];
static volatile int g_ringWrite = 0;
static volatile int g_ringRead  = 0;
static const int    RING_CAP    = (int)(IX_VID_RING_SIZE / sizeof(float));

static void Ring_Reset()  { g_ringWrite = 0; g_ringRead = 0; }

static void Ring_Write(const float* src, int count)
{
    for (int i = 0; i < count; i++)
    {
        int next = (g_ringWrite + 1) % RING_CAP;
        if (next == g_ringRead) break;      // cheio — descarta
        g_ringBuf[g_ringWrite] = src[i];
        g_ringWrite = next;
    }
}

static int Ring_Read(float* dst, int count)
{
    int read = 0;
    while (read < count && g_ringRead != g_ringWrite)
    {
        dst[read++] = g_ringBuf[g_ringRead];
        g_ringRead = (g_ringRead + 1) % RING_CAP;
    }
    // Silence para frames faltantes
    while (read < count) dst[read++] = 0.0f;
    return read;
}

// ---------------------------------------------------------------------------
//  Estado interno
// ---------------------------------------------------------------------------
namespace
{
    static plm_t*              g_plm        = NULL;
    static IDirect3DTexture9*  g_texture    = NULL;
    static int                 g_texW       = 0;
    static int                 g_texH       = 0;

    // Pixel buffer ARGB para upload D3D9
    static unsigned int*       g_pixels     = NULL;
    static int                 g_pixW       = 0;
    static int                 g_pixH       = 0;
    static int                 g_newFrame   = 0;

    // Estado de reproducao
    static int                 g_open       = 0;
    static int                 g_playing    = 0;
    static int                 g_paused     = 0;
    static int                 g_finished   = 0;
    static int                 g_loop       = 0;
    static float               g_volume     = 1.0f;
    static float               g_videoTime  = 0.0f;  // tempo acumulado

    // Audio device miniaudio dedicado ao video
    static ma_device           g_audioDevice;
    static int                 g_audioOpen  = 0;
    static int                 g_audioSampleRate = 44100;

    // Erro
    static char                g_lastError[256] = "VideoPlayer offline";
    static void SetError(const char* s)
    {
        snprintf(g_lastError, sizeof(g_lastError), "%s", s ? s : "Unknown");
    }

    // -----------------------------------------------------------------------
    //  Callbacks pl_mpeg
    // -----------------------------------------------------------------------
    static void OnVideoFrame(plm_t* plm, plm_frame_t* frame, void* user)
    {
        (void)plm; (void)user;
        if (!frame || !g_pixels) return;

        // pl_mpeg escreve RGBA (R,G,B,A em memoria)
        // D3DFMT_A8R8G8B8 espera BGRA (B,G,R,A em memoria) no little-endian
        // Entao: converte RGBA -> troca byte 0 (R) com byte 2 (B) -> BGRA
        plm_frame_to_rgba(frame, (uint8_t*)g_pixels, g_pixW * 4);
        int total = g_pixW * g_pixH;
        for (int i = 0; i < total; i++)
        {
            unsigned int p = g_pixels[i];
            // p = 0xAABBGGRR em little-endian apos rgba = RRGGBBAA em bytes
            // swap: move R (bits 0-7) para bits 16-23 e B (bits 16-23) para bits 0-7
            g_pixels[i] = (p & 0xFF00FF00u)
                        | ((p & 0x000000FFu) << 16)   // R -> posicao B
                        | ((p & 0x00FF0000u) >> 16)   // B -> posicao R
                        | 0xFF000000u;                 // alpha sempre opaco
        }
        g_newFrame = 1;
    }

    static void OnAudioSamples(plm_t* plm, plm_samples_t* samples, void* user)
    {
        (void)plm; (void)user;
        if (!samples) return;
        Ring_Write(samples->interleaved, samples->count * 2);
    }

    // -----------------------------------------------------------------------
    //  miniaudio callback — chamado em thread separada pelo driver de audio
    // -----------------------------------------------------------------------
    static void AudioDataCallback(ma_device* device, void* output, const void* input,
                                   ma_uint32 frameCount)
    {
        (void)device; (void)input;
        float* out = (float*)output;
        Ring_Read(out, (int)frameCount * 2);

        // Aplica volume
        if (g_volume < 0.9999f)
        {
            int total = (int)frameCount * 2;
            for (int i = 0; i < total; i++)
                out[i] *= g_volume;
        }
    }

    // -----------------------------------------------------------------------
    //  Abre device de audio miniaudio para o video
    // -----------------------------------------------------------------------
    static int Audio_Open(int sampleRate)
    {
        if (g_audioOpen) return 1;

        ma_device_config cfg = ma_device_config_init(ma_device_type_playback);
        cfg.playback.format   = ma_format_f32;
        cfg.playback.channels = 2;
        cfg.sampleRate        = (ma_uint32)sampleRate;
        cfg.dataCallback      = AudioDataCallback;

        if (ma_device_init(NULL, &cfg, &g_audioDevice) != MA_SUCCESS)
        {
            SetError("VideoPlayer: audio device init failed");
            return 0;
        }
        ma_device_start(&g_audioDevice);
        g_audioOpen = 1;
        return 1;
    }

    static void Audio_Close()
    {
        if (!g_audioOpen) return;
        ma_device_stop(&g_audioDevice);
        ma_device_uninit(&g_audioDevice);
        g_audioOpen = 0;
    }

    // -----------------------------------------------------------------------
    //  Aloca pixel buffer e textura D3D9
    // -----------------------------------------------------------------------
    static int AllocResources(int w, int h)
    {
        if (g_pixels && g_pixW == w && g_pixH == h) return 1;

        // Libera anterior
        if (g_pixels) { delete[] g_pixels; g_pixels = NULL; }
        if (g_texture) { g_texture->Release(); g_texture = NULL; }

        g_pixels = new unsigned int[(size_t)w * h];
        if (!g_pixels) { SetError("VideoPlayer: out of memory"); return 0; }
        memset(g_pixels, 0, (size_t)w * h * 4);
        g_pixW = w; g_pixH = h;

        IDirect3DDevice9* device = Renderer_GetDevice();
        if (!device) { SetError("VideoPlayer: no D3D9 device"); return 0; }

        HRESULT hr = device->CreateTexture(
            (UINT)w, (UINT)h, 1, D3DUSAGE_DYNAMIC,
            D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
            &g_texture, NULL);

        if (FAILED(hr)) { SetError("VideoPlayer: texture create failed"); return 0; }

        g_texW = w; g_texH = h;
        return 1;
    }

    static void FreeResources()
    {
        if (g_pixels)  { delete[] g_pixels;     g_pixels  = NULL; }
        if (g_texture) { g_texture->Release();  g_texture = NULL; }
        g_pixW = g_pixH = g_texW = g_texH = 0;
    }
}

// ---------------------------------------------------------------------------
//  API publica
// ---------------------------------------------------------------------------
int VideoPlayer_Open(const char* path)
{
    VideoPlayer_Close();

    if (!path || !path[0]) { SetError("VideoPlayer: null path"); return 0; }

    g_plm = plm_create_with_filename(path);
    if (!g_plm) { SetError("VideoPlayer: cannot open file"); return 0; }

    // Verifica que tem stream de video
    if (!plm_get_num_video_streams(g_plm))
    {
        plm_destroy(g_plm); g_plm = NULL;
        SetError("VideoPlayer: no video stream");
        return 0;
    }

    int w = plm_get_width(g_plm);
    int h = plm_get_height(g_plm);
    if (w <= 0 || h <= 0)
    {
        plm_destroy(g_plm); g_plm = NULL;
        SetError("VideoPlayer: invalid dimensions");
        return 0;
    }

    if (!AllocResources(w, h))
    {
        plm_destroy(g_plm); g_plm = NULL;
        return 0;
    }

    // Configura callbacks
    plm_set_video_decode_callback(g_plm, OnVideoFrame, NULL);

    // Audio (opcional — pode nao ter stream de audio)
    if (plm_get_num_audio_streams(g_plm) > 0)
    {
        g_audioSampleRate = plm_get_samplerate(g_plm);
        plm_set_audio_decode_callback(g_plm, OnAudioSamples, NULL);
        plm_set_audio_lead_time(g_plm, 0.15);  // buffer de 150ms
        Ring_Reset();
        Audio_Open(g_audioSampleRate);
    }

    plm_set_loop(g_plm, 0);  // gerenciamos loop manualmente

    g_open      = 1;
    g_playing   = 1;
    g_paused    = 0;
    g_finished  = 0;
    g_videoTime = 0.0f;
    g_newFrame  = 0;

    snprintf(g_lastError, sizeof(g_lastError), "VideoPlayer: open ok (%dx%d)", w, h);
    return 1;
}

void VideoPlayer_Close()
{
    if (g_plm)  { plm_destroy(g_plm); g_plm = NULL; }
    Audio_Close();
    FreeResources();
    Ring_Reset();
    g_open = g_playing = g_paused = g_finished = 0;
    g_videoTime = 0.0f;
}

void VideoPlayer_Play()    { if (g_open) { g_playing=1; g_paused=0; } }
void VideoPlayer_Pause()   { if (g_open) { g_playing=0; g_paused=1; } }
void VideoPlayer_SetLoop(int loop) { g_loop = loop ? 1 : 0; }

void VideoPlayer_Stop()
{
    if (!g_open) return;
    g_playing  = 0;
    g_paused   = 0;
    g_finished = 0;
    g_videoTime= 0.0f;
    if (g_plm) plm_rewind(g_plm);
    Ring_Reset();
}

void VideoPlayer_Update(float deltaSeconds)
{
    if (!g_open || !g_playing || g_paused || g_finished) return;
    if (!g_plm) return;

    // Avanca a decodificacao pelo tempo decorrido
    // plm_decode dispara os callbacks OnVideoFrame e OnAudioSamples
    plm_decode(g_plm, (double)deltaSeconds);

    if (plm_has_ended(g_plm))
    {
        if (g_loop)
        {
            plm_rewind(g_plm);
            Ring_Reset();
            g_videoTime = 0.0f;
        }
        else
        {
            g_finished = 1;
            g_playing  = 0;
        }
    }
}

void VideoPlayer_Render(int x, int y, int w, int h)
{
    if (!g_open || !g_texture || !g_pixels) return;

    // Upload do frame novo para a textura D3D9
    if (g_newFrame)
    {
        D3DLOCKED_RECT locked;
        if (SUCCEEDED(g_texture->LockRect(0, &locked, NULL, D3DLOCK_DISCARD)))
        {
            for (int row = 0; row < g_pixH; row++)
            {
                unsigned int* dst = (unsigned int*)((unsigned char*)locked.pBits + row * locked.Pitch);
                unsigned int* src = g_pixels + row * g_pixW;
                memcpy(dst, src, (size_t)g_pixW * 4);
            }
            g_texture->UnlockRect(0);
        }
        g_newFrame = 0;
    }

    GUI_DrawSprite(g_texture, 0, 0, g_texW, g_texH, x, y, w, h, 0xFFFFFFFF);
}

void VideoPlayer_RenderFit(int x, int y, int w, int h, unsigned int fillColor)
{
    if (!g_open || !g_texture) return;

    // Barras de letterbox/pillarbox
    if ((fillColor >> 24) & 0xFF)
        GUI_DrawFilledRect(x, y, w, h, fillColor);

    if (g_texW <= 0 || g_texH <= 0) return;

    float scaleX = (float)w / (float)g_texW;
    float scaleY = (float)h / (float)g_texH;
    float scale  = scaleX < scaleY ? scaleX : scaleY;
    int dw = (int)((float)g_texW * scale);
    int dh = (int)((float)g_texH * scale);
    int dx = x + (w - dw) / 2;
    int dy = y + (h - dh) / 2;

    VideoPlayer_Render(dx, dy, dw, dh);
}

int   VideoPlayer_IsOpen()       { return g_open; }
int   VideoPlayer_IsPlaying()    { return g_open && g_playing && !g_paused; }
int   VideoPlayer_IsPaused()     { return g_open && g_paused; }
int   VideoPlayer_IsFinished()   { return g_finished; }
int   VideoPlayer_GetVideoWidth()  { return g_texW; }
int   VideoPlayer_GetVideoHeight() { return g_texH; }

float VideoPlayer_GetDuration()
{
    if (!g_plm) return 0.0f;
    return (float)plm_get_duration(g_plm);
}
float VideoPlayer_GetPosition()
{
    if (!g_plm) return 0.0f;
    return (float)plm_get_time(g_plm);
}

void  VideoPlayer_SetVolume(float v) { g_volume = v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }
float VideoPlayer_GetVolume()        { return g_volume; }

const char* VideoPlayer_GetLastError() { return g_lastError; }

// ---------------------------------------------------------------------------
//  Modo bloqueante
// ---------------------------------------------------------------------------
void VideoPlayer_PlayBlocking(HWND hWnd, const char* path,
                               int loop, int hideOverlay)
{
    if (!VideoPlayer_Open(path)) return;
    VideoPlayer_SetLoop(loop);

    if (hideOverlay) Renderer_SetOverlayEnabled(0);

    LARGE_INTEGER freq, prev, now;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&prev);

    while (!VideoPlayer_IsFinished())
    {
        // Processa mensagens
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                PostQuitMessage(0);
                goto done;
            }
            if (msg.message == WM_KEYDOWN)
            {
                WPARAM k = msg.wParam;
                if (k == VK_ESCAPE || k == VK_RETURN || k == VK_SPACE)
                    goto done;
            }
            if (msg.message == WM_LBUTTONDOWN)
                goto done;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Delta time
        QueryPerformanceCounter(&now);
        float dt = (float)(now.QuadPart - prev.QuadPart) / (float)freq.QuadPart;
        if (dt > 0.1f) dt = 0.1f;
        prev = now;

        // Update + render
        VideoPlayer_Update(dt);

        Renderer_BeginFrame();
        int scrW = 0, scrH = 0;
        Renderer_GetClientSize(&scrW, &scrH);
        VideoPlayer_RenderFit(0, 0, scrW, scrH, 0xFF000000);
        Renderer_EndFrame();

        Sleep(1);
    }

done:
    if (hideOverlay) Renderer_SetOverlayEnabled(1);
    VideoPlayer_Close();
}
