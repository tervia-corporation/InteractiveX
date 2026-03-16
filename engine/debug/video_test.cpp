#include "video_test.h"
#include "../renderer.h"
#include "../gui.h"
#include "../core/input.h"
#include "../core/timer.h"
#include "../media/video_player.h"
#include "../interface/button.h"
#include "../interface/slider.h"
#include "../interface/layout.h"
#include <cstdio>

// ---------------------------------------------------------------------------
//  Video Test
//
//  Demonstra o VideoPlayer com um arquivo de teste.
//  Coloque um .mpg em: assets/video/test.mpg
//  Para converter com ffmpeg:
//      ffmpeg -i input.mp4 -c:v mpeg1video -c:a mp2 -q:v 5 assets/video/test.mpg
//
//  Botoes:
//      Play/Pause   -> alterna reproducao
//      Stop         -> para e volta ao inicio
//      Loop         -> ativa/desativa loop
//
//  Slider:
//      Volume       -> volume do audio do video
// ---------------------------------------------------------------------------

#define VIDEO_PATH "assets/video/test.mpg"

namespace
{
    static IXButton g_btnPlayPause;
    static IXButton g_btnStop;
    static IXButton g_btnLoop;
    static const IXButton* g_buttons[3];

    static IXSlider g_sliderVolume;
    static const IXSlider* g_sliders[1];

    static int  g_returnToMenu = 0;
    static int  g_loop         = 0;
    static char g_statusLine[256] = "Video test ready.";
    static char g_helpLine[256]   = "Place assets/video/test.mpg | ESC menu";

    struct VLayoutCache { int valid, x, y, w, h; };
    static VLayoutCache g_cache = {0,0,0,0,0};

    static void RefreshLayout()
    {
        int x, y, w, h;
        Renderer_GetOverlayContentRect(&x, &y, &w, &h);
        g_cache = {1, x, y, w, h};

        IXLayout layout;
        Layout_BeginVertical(&layout, x, y + 96, w, h - 96, 8);

        int bx, by, bw, bh;
        Layout_Next(&layout, 36, &bx, &by, &bw, &bh); Button_SetBounds(&g_btnPlayPause, bx, by, bw, bh);
        Layout_Next(&layout, 36, &bx, &by, &bw, &bh); Button_SetBounds(&g_btnStop,      bx, by, bw, bh);
        Layout_Next(&layout, 36, &bx, &by, &bw, &bh); Button_SetBounds(&g_btnLoop,      bx, by, bw, bh);

        Layout_Skip(&layout, 10);
        Layout_Next(&layout, 40, &bx, &by, &bw, &bh); Slider_SetBounds(&g_sliderVolume, bx, by, bw, bh);
    }

    static void EnsureLayout()
    {
        int x, y, w, h;
        Renderer_GetOverlayContentRect(&x, &y, &w, &h);
        if (!g_cache.valid || g_cache.x!=x || g_cache.y!=y || g_cache.w!=w || g_cache.h!=h)
            RefreshLayout();
    }

    static void UpdateLoopButtonLabel()
    {
        Button_SetText(&g_btnLoop, g_loop ? "Loop: ON" : "Loop: OFF");
    }

    static void UpdatePlayPauseLabel()
    {
        if (!VideoPlayer_IsOpen())
            Button_SetText(&g_btnPlayPause, "Play");
        else if (VideoPlayer_IsPlaying())
            Button_SetText(&g_btnPlayPause, "Pause");
        else
            Button_SetText(&g_btnPlayPause, "Resume");
    }

    static void UpdateStatus()
    {
        if (!VideoPlayer_IsOpen())
        {
            std::snprintf(g_statusLine, sizeof(g_statusLine),
                "No video open. %s", VideoPlayer_GetLastError());
            return;
        }
        float pos = VideoPlayer_GetPosition();
        float dur = VideoPlayer_GetDuration();
        int   fw  = VideoPlayer_GetVideoWidth();
        int   fh  = VideoPlayer_GetVideoHeight();
        const char* state = VideoPlayer_IsFinished() ? "Finished"
                          : VideoPlayer_IsPlaying()  ? "Playing"
                          : "Paused";
        std::snprintf(g_statusLine, sizeof(g_statusLine),
            "%s | %.1fs / %.1fs | %dx%d", state, pos, dur, fw, fh);
    }
}

// ---------------------------------------------------------------------------
//  API publica
// ---------------------------------------------------------------------------
void DebugVideoTest_Initialize()
{
    Button_Init(&g_btnPlayPause, 0, 0, 0, 0, "Play");
    Button_Init(&g_btnStop,      0, 0, 0, 0, "Stop");
    Button_Init(&g_btnLoop,      0, 0, 0, 0, "Loop: OFF");

    Button_SetColors(&g_btnPlayPause, 0xCC1E3828, 0xCC2A5038, 0xCC386848);
    Button_SetColors(&g_btnStop,      0xCC4A1E1E, 0xCC682828, 0xCC883434);
    Button_SetColors(&g_btnLoop,      0xCC2A2A18, 0xCC3E3E24, 0xCC545432);

    g_buttons[0] = &g_btnPlayPause;
    g_buttons[1] = &g_btnStop;
    g_buttons[2] = &g_btnLoop;

    Slider_Init(&g_sliderVolume, 0, 0, 0, 0, 0, 100, 100, "Volume");
    g_sliders[0] = &g_sliderVolume;

    g_returnToMenu = 0;
    g_loop         = 0;
    g_cache.valid  = 0;

    // Tenta abrir o video automaticamente
    if (VideoPlayer_Open(VIDEO_PATH))
    {
        VideoPlayer_SetLoop(g_loop);
        std::snprintf(g_statusLine, sizeof(g_statusLine), "Opened: %s", VIDEO_PATH);
    }
    else
    {
        std::snprintf(g_statusLine, sizeof(g_statusLine),
            "Cannot open %s", VIDEO_PATH);
    }

    UpdatePlayPauseLabel();
    UpdateLoopButtonLabel();
    EnsureLayout();
}

void DebugVideoTest_InvalidateLayout() { g_cache.valid = 0; }

void DebugVideoTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased)
{
    EnsureLayout();

    if (Input_WasKeyPressed(VK_ESCAPE))
    {
        VideoPlayer_Close();
        g_returnToMenu = 1;
        return;
    }

    // Sincroniza volume
    int prevVol = Slider_GetValue(&g_sliderVolume);
    Slider_Handle(&g_sliderVolume, mouseX, mouseY, mouseDown);
    if (Slider_GetValue(&g_sliderVolume) != prevVol)
        VideoPlayer_SetVolume((float)Slider_GetValue(&g_sliderVolume) / 100.0f);

    // Botao Play/Pause
    if (Button_Handle(&g_btnPlayPause, mouseX, mouseY, mouseDown, mouseReleased))
    {
        if (!VideoPlayer_IsOpen())
        {
            if (VideoPlayer_Open(VIDEO_PATH))
            {
                VideoPlayer_SetLoop(g_loop);
                VideoPlayer_SetVolume((float)Slider_GetValue(&g_sliderVolume) / 100.0f);
            }
        }
        else if (VideoPlayer_IsPlaying())
            VideoPlayer_Pause();
        else
            VideoPlayer_Play();
        UpdatePlayPauseLabel();
    }

    // Botao Stop
    if (Button_Handle(&g_btnStop, mouseX, mouseY, mouseDown, mouseReleased))
    {
        VideoPlayer_Stop();
        UpdatePlayPauseLabel();
    }

    // Botao Loop
    if (Button_Handle(&g_btnLoop, mouseX, mouseY, mouseDown, mouseReleased))
    {
        g_loop = !g_loop;
        VideoPlayer_SetLoop(g_loop);
        UpdateLoopButtonLabel();
    }

    // Atualiza decodificacao
    if (VideoPlayer_IsPlaying())
        VideoPlayer_Update(Timer_GetDeltaSeconds());

    // Se terminou sem loop, atualiza label
    if (VideoPlayer_IsFinished())
        UpdatePlayPauseLabel();

    UpdateStatus();
}

void DebugVideoTest_Render()
{
    if (!VideoPlayer_IsOpen()) return;

    // Renderiza o frame na area a direita do overlay
    int cw = 0, ch = 0;
    Renderer_GetClientSize(&cw, &ch);
    const int VX = 392;
    int vw = cw - VX - 8;
    int vh = ch - 8;
    if (vw < 16 || vh < 16) return;

    VideoPlayer_RenderFit(VX, 4, vw, vh, 0xFF000000);
}

void DebugVideoTest_ApplyRendererState(float renderFPS, float frameMs)
{
    EnsureLayout();
    Renderer_SetOverlayText("InteractiveX r0.2", "Video test", g_statusLine);
    Renderer_SetHelpText(g_helpLine);
    Renderer_SetDisplayOptions(1, 1, 0);
    Renderer_SetControlButton(NULL);
    Renderer_SetControlButtons(g_buttons, 3);
    Renderer_SetControlSliders(g_sliders, 1);
    Renderer_SetOverlayMetrics(renderFPS, frameMs, 0.0f);
}

int  DebugVideoTest_ShouldReturnToMenu() { return g_returnToMenu; }
void DebugVideoTest_ClearReturnToMenu()  { g_returnToMenu = 0;    }
