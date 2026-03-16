#include "audio_test.h"
#include "../renderer.h"
#include "../core/input.h"
#include "../interface/button.h"
#include "../interface/layout.h"
#include "../media/audio.h"
#include <cstdio>
#include <cstring>

namespace
{
    static IXButton g_playWavButton;
    static IXButton g_loopWavButton;
    static IXButton g_pauseResumeButton;
    static IXButton g_stopButton;
    static const IXButton* g_buttons[4];

    static int  g_returnToMenu = 0;
    static char g_statusLine[256] = "Audio test ready.";
    static char g_helpLine[256]   = "assets/demo.wav | ESC menu";

    struct AudioLayoutCache { int valid, x, y, width, height; };
    static AudioLayoutCache g_layoutCache = {0, 0, 0, 0, 0};

    static void RefreshLayout()
    {
        IXLayout layout;
        int x, y, width, height;
        Renderer_GetOverlayContentRect(&x, &y, &width, &height);
        g_layoutCache = { 1, x, y, width, height };

        Layout_BeginVertical(&layout, x, y + 88, width, height - 88, 12);
        Layout_Next(&layout, 38, &x, &y, &width, &height); Button_SetBounds(&g_playWavButton,      x, y, width, height);
        Layout_Next(&layout, 38, &x, &y, &width, &height); Button_SetBounds(&g_loopWavButton,      x, y, width, height);
        Layout_Next(&layout, 38, &x, &y, &width, &height); Button_SetBounds(&g_pauseResumeButton,  x, y, width, height);
        Layout_Next(&layout, 38, &x, &y, &width, &height); Button_SetBounds(&g_stopButton,         x, y, width, height);
    }

    static void EnsureLayout()
    {
        int x, y, w, h;
        Renderer_GetOverlayContentRect(&x, &y, &w, &h);
        if (!g_layoutCache.valid ||
            g_layoutCache.x != x || g_layoutCache.y != y ||
            g_layoutCache.width != w || g_layoutCache.height != h)
            RefreshLayout();
    }

    static void UpdatePauseResumeLabel()
    {
        Button_SetText(&g_pauseResumeButton, MediaAudio_IsPaused() ? "Resume audio" : "Pause audio");
    }

    static void RefreshStatus(const char* prefix)
    {
        const char* path  = MediaAudio_GetLastResolvedPath();
        const char* state = MediaAudio_GetLastError();
        if (!prefix) prefix = "Audio";
        std::snprintf(g_statusLine, sizeof(g_statusLine), "%s | %s", prefix, state ? state : "Unknown");
        std::snprintf(g_helpLine,   sizeof(g_helpLine),
            "%s%s%s | ESC menu",
            path && path[0] ? path : "No file resolved",
            MediaAudio_HasActiveFile() ? " | active" : " | idle",
            MediaAudio_IsPaused()      ? " | paused" : "");
    }
}

void DebugAudioTest_Initialize()
{
    Button_Init(&g_playWavButton,     28, 140, 220, 38, "Play demo.wav");
    Button_Init(&g_loopWavButton,     28, 190, 220, 38, "Loop demo.wav");
    Button_Init(&g_pauseResumeButton, 28, 240, 220, 38, "Pause audio");
    Button_Init(&g_stopButton,        28, 290, 220, 38, "Stop audio");

    Button_SetColors(&g_loopWavButton, 0xCC244131, 0xCC2E5742, 0xCC387055);
    Button_SetColors(&g_pauseResumeButton, 0xCC4A3C1C, 0xCC665326, 0xCC876C2F);
    Button_SetColors(&g_stopButton, 0xCC4A2525, 0xCC663232, 0xCC884141);

    g_buttons[0] = &g_playWavButton;
    g_buttons[1] = &g_loopWavButton;
    g_buttons[2] = &g_pauseResumeButton;
    g_buttons[3] = &g_stopButton;

    g_returnToMenu      = 0;
    g_layoutCache.valid = 0;
    EnsureLayout();
    MediaAudio_Stop();
    UpdatePauseResumeLabel();
    RefreshStatus("Audio test ready");
}

void DebugAudioTest_InvalidateLayout()
{
    g_layoutCache.valid = 0;
}

void DebugAudioTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased)
{
    EnsureLayout();

    if (Input_WasKeyPressed(VK_ESCAPE))
    {
        MediaAudio_Stop();
        g_returnToMenu = 1;
        return;
    }

    if (Button_Handle(&g_playWavButton, mouseX, mouseY, mouseDown, mouseReleased))
    {
        MediaAudio_PlayDemoWAV(0);
        UpdatePauseResumeLabel();
        RefreshStatus("Play WAV");
    }
    if (Button_Handle(&g_loopWavButton, mouseX, mouseY, mouseDown, mouseReleased))
    {
        MediaAudio_PlayDemoWAV(1);
        UpdatePauseResumeLabel();
        RefreshStatus("Loop WAV");
    }
    if (Button_Handle(&g_pauseResumeButton, mouseX, mouseY, mouseDown, mouseReleased))
    {
        if (MediaAudio_HasActiveFile())
            MediaAudio_IsPaused() ? MediaAudio_Resume() : MediaAudio_Pause();
        UpdatePauseResumeLabel();
        RefreshStatus(MediaAudio_IsPaused() ? "Paused" : "Resumed");
    }
    if (Button_Handle(&g_stopButton, mouseX, mouseY, mouseDown, mouseReleased))
    {
        MediaAudio_Stop();
        UpdatePauseResumeLabel();
        RefreshStatus("Stopped");
    }
}

void DebugAudioTest_ApplyRendererState(float renderFPS, float frameMs)
{
    EnsureLayout();
    Renderer_SetOverlayText("InteractiveX r0.2", "Audio test", g_statusLine);
    Renderer_SetHelpText(g_helpLine);
    Renderer_SetDisplayOptions(1, 1, 1);
    Renderer_SetControlButton(NULL);
    Renderer_SetControlButtons(g_buttons, 4);
    Renderer_SetControlSliders(NULL, 0);
    Renderer_SetOverlayMetrics(renderFPS, frameMs, 0.0f);
}

int  DebugAudioTest_ShouldReturnToMenu() { return g_returnToMenu; }
void DebugAudioTest_ClearReturnToMenu()  { g_returnToMenu = 0;    }
