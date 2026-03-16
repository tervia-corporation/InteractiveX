#include "random_test.h"
#include "../renderer.h"
#include "../gui.h"
#include "../core/input.h"
#include "../core/random.h"
#include "../interface/button.h"
#include "../interface/slider.h"
#include "../interface/layout.h"
#include <cstdio>

// ---------------------------------------------------------------------------
//  Random Test
//
//  Visualiza a distribuicao do RNG desenhando pontos na area de viewport.
//
//  Botoes:
//      Reseed (time)   -> Rng_SeedFromTime: seed nao deterministica
//      Reseed (fixed)  -> seed fixa (0xDEADBEEF): mesma sequencia sempre
//      Shuffle         -> embaralha array [0..N] e mostra ordem resultante
//
//  Sliders:
//      Seed value      -> seed manual (0 a 99999)
//      Point count     -> quantos pontos plotar (100 a 2000)
//
//  Visualizacao:
//      Pontos brancos  -> Rng_Float em [0,1) para X e Y no viewport
//      Texto no canto  -> primeiros 8 valores Rng_Int(1,100) da sequencia
// ---------------------------------------------------------------------------

#define MAX_POINTS 2000
#define SHUFFLE_N  16

namespace
{
    // Controles
    static IXButton g_btnReseedTime;
    static IXButton g_btnReseedFixed;
    static IXButton g_btnShuffle;
    static const IXButton* g_buttons[3];

    static IXSlider g_sliderSeed;
    static IXSlider g_sliderCount;
    static const IXSlider* g_sliders[2];

    // Estado
    static IXRng    g_rng;
    static int      g_returnToMenu = 0;
    static char     g_statusLine[256] = "Random test ready.";
    static char     g_helpLine[256]   = "Reseed to regenerate | ESC menu";

    // Pontos gerados
    static float    g_pointsX[MAX_POINTS];
    static float    g_pointsY[MAX_POINTS];
    static int      g_pointCount = 0;

    // Amostra dos primeiros ints gerados (para exibir numericamente)
    static int      g_sample[8];
    static int      g_hasSample = 0;

    // Shuffle demo
    static int      g_shuffleArr[SHUFFLE_N];
    static int      g_hasShuffle = 0;

    // Layout cache
    struct RLayoutCache { int valid, x, y, w, h; };
    static RLayoutCache g_cache = {0,0,0,0,0};

    static void RefreshLayout()
    {
        int x, y, w, h;
        Renderer_GetOverlayContentRect(&x, &y, &w, &h);
        g_cache = {1, x, y, w, h};

        IXLayout layout;
        Layout_BeginVertical(&layout, x, y + 96, w, h - 96, 8);

        int bx, by, bw, bh;
        Layout_Next(&layout, 36, &bx, &by, &bw, &bh); Button_SetBounds(&g_btnReseedTime,  bx, by, bw, bh);
        Layout_Next(&layout, 36, &bx, &by, &bw, &bh); Button_SetBounds(&g_btnReseedFixed, bx, by, bw, bh);
        Layout_Next(&layout, 36, &bx, &by, &bw, &bh); Button_SetBounds(&g_btnShuffle,     bx, by, bw, bh);

        Layout_Skip(&layout, 10);
        Layout_Next(&layout, 40, &bx, &by, &bw, &bh); Slider_SetBounds(&g_sliderSeed,  bx, by, bw, bh);
        Layout_Next(&layout, 40, &bx, &by, &bw, &bh); Slider_SetBounds(&g_sliderCount, bx, by, bw, bh);
    }

    static void EnsureLayout()
    {
        int x, y, w, h;
        Renderer_GetOverlayContentRect(&x, &y, &w, &h);
        if (!g_cache.valid || g_cache.x != x || g_cache.y != y || g_cache.w != w || g_cache.h != h)
            RefreshLayout();
    }

    // Gera pontos e amostra com o estado atual do rng
    static void GeneratePoints()
    {
        int count = Slider_GetValue(&g_sliderCount);
        if (count > MAX_POINTS) count = MAX_POINTS;
        g_pointCount = count;

        // Copia o rng para nao consumir o estado original ao gerar a amostra
        IXRng tmp = g_rng;
        for (int i = 0; i < count; i++)
        {
            g_pointsX[i] = Rng_NextFloat01(&tmp);
            g_pointsY[i] = Rng_NextFloat01(&tmp);
        }

        // Amostra numerica: recomeca do estado original
        tmp = g_rng;
        for (int i = 0; i < 8; i++)
            g_sample[i] = Rng_Int(&tmp, 1, 100);
        g_hasSample = 1;
    }

    static void DoReseedTime()
    {
        Rng_SeedFromTime(&g_rng);
        GeneratePoints();
        g_hasShuffle = 0;
        std::snprintf(g_statusLine, sizeof(g_statusLine),
            "Seeded from time. %d points plotted.", g_pointCount);
    }

    static void DoReseedFixed()
    {
        unsigned int seed = (unsigned int)Slider_GetValue(&g_sliderSeed);
        Rng_Seed(&g_rng, seed);
        GeneratePoints();
        g_hasShuffle = 0;
        std::snprintf(g_statusLine, sizeof(g_statusLine),
            "Seed=%u  ->  %d points plotted. (reproducible)", seed, g_pointCount);
    }

    static void DoShuffle()
    {
        unsigned int seed = (unsigned int)Slider_GetValue(&g_sliderSeed);
        IXRng tmp;
        Rng_Seed(&tmp, seed);
        for (int i = 0; i < SHUFFLE_N; i++) g_shuffleArr[i] = i;
        Rng_ShuffleInts(&tmp, g_shuffleArr, SHUFFLE_N);
        g_hasShuffle = 1;
        std::snprintf(g_statusLine, sizeof(g_statusLine),
            "Shuffled [0..%d] with seed=%u.", SHUFFLE_N - 1, seed);
    }
}

// ---------------------------------------------------------------------------
//  API publica
// ---------------------------------------------------------------------------

void DebugRandomTest_Initialize()
{
    Button_Init(&g_btnReseedTime,  0, 0, 0, 0, "Reseed (time)");
    Button_Init(&g_btnReseedFixed, 0, 0, 0, 0, "Reseed (slider seed)");
    Button_Init(&g_btnShuffle,     0, 0, 0, 0, "Shuffle [0..15]");

    Button_SetColors(&g_btnReseedTime, 0xCC1E3828, 0xCC2A5038, 0xCC386848);
    Button_SetColors(&g_btnReseedFixed, 0xCC1E2840, 0xCC284060, 0xCC345880);
    Button_SetColors(&g_btnShuffle, 0xCC382818, 0xCC503820, 0xCC6A4C2C);

    g_buttons[0] = &g_btnReseedTime;
    g_buttons[1] = &g_btnReseedFixed;
    g_buttons[2] = &g_btnShuffle;

    Slider_Init(&g_sliderSeed,  0, 0, 0, 0, 0,   99999, 42,   "Seed value");
    Slider_Init(&g_sliderCount, 0, 0, 0, 0, 100, 2000,  800,  "Point count");

    g_sliders[0] = &g_sliderSeed;
    g_sliders[1] = &g_sliderCount;

    g_returnToMenu  = 0;
    g_hasSample     = 0;
    g_hasShuffle    = 0;
    g_cache.valid   = 0;

    // Gera distribuicao inicial com seed padrao
    Rng_Seed(&g_rng, 42);
    GeneratePoints();

    std::snprintf(g_statusLine, sizeof(g_statusLine), "Seed=42 | %d points plotted.", g_pointCount);
    std::snprintf(g_helpLine,   sizeof(g_helpLine),   "Same seed = same sequence | ESC menu");
    EnsureLayout();
}

void DebugRandomTest_InvalidateLayout()
{
    g_cache.valid = 0;
}

void DebugRandomTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased)
{
    EnsureLayout();

    if (Input_WasKeyPressed(VK_ESCAPE)) { g_returnToMenu = 1; return; }

    // Regenera pontos quando o slider de contagem muda
    int prevCount = g_pointCount;
    Slider_Handle(&g_sliderSeed,  mouseX, mouseY, mouseDown);
    Slider_Handle(&g_sliderCount, mouseX, mouseY, mouseDown);
    if (Slider_GetValue(&g_sliderCount) != prevCount) GeneratePoints();

    if (Button_Handle(&g_btnReseedTime,  mouseX, mouseY, mouseDown, mouseReleased)) DoReseedTime();
    if (Button_Handle(&g_btnReseedFixed, mouseX, mouseY, mouseDown, mouseReleased)) DoReseedFixed();
    if (Button_Handle(&g_btnShuffle,     mouseX, mouseY, mouseDown, mouseReleased)) DoShuffle();
}

void DebugRandomTest_Render()
{
    int cw, ch;
    Renderer_GetClientSize(&cw, &ch);

    // Viewport: area a direita do overlay (x=392 como nos outros testes)
    const int VX = 392;
    const int VY = 24;
    int vw = cw - VX - 16;
    int vh = ch - VY - 24;
    if (vw < 32 || vh < 32) return;

    // Fundo da area de plotagem
    GUI_DrawFilledRect(VX, VY, vw, vh, 0x28000000);
    GUI_DrawOutlinedRect(VX, VY, vw, vh, 0x33FFFFFF);

    // Pontos
    for (int i = 0; i < g_pointCount; i++)
    {
        int px = VX + (int)(g_pointsX[i] * (float)(vw - 2));
        int py = VY + (int)(g_pointsY[i] * (float)(vh - 2));
        // Cores ciclicas suaves para facilitar a leitura visual da distribuicao
        unsigned int hue = (unsigned int)(i * 7919u) & 0xFFFFFF;
        unsigned int col = 0xCC000000 | hue;
        GUI_DrawFilledRect(px, py, 2, 2, col);
    }

    // Legenda: primeiros 8 valores inteiros [1,100]
    if (g_hasSample)
    {
        char buf[128];
        std::snprintf(buf, sizeof(buf),
            "Int[1,100]: %d %d %d %d %d %d %d %d",
            g_sample[0], g_sample[1], g_sample[2], g_sample[3],
            g_sample[4], g_sample[5], g_sample[6], g_sample[7]);
        GUI_DrawFilledRect(VX + 4, VY + vh - 22, vw - 8, 18, 0x44000000);
        GUI_DrawTextLine(VX + 8, VY + vh - 22, buf, 0xFFCCDD88);
    }

    // Resultado do shuffle
    if (g_hasShuffle)
    {
        char buf[256];
        char* p = buf;
        int   rem = (int)sizeof(buf);
        int   n = std::snprintf(p, rem, "Shuffle: ");
        p += n; rem -= n;
        for (int i = 0; i < SHUFFLE_N && rem > 4; i++)
        {
            n = std::snprintf(p, rem, "%d ", g_shuffleArr[i]);
            p += n; rem -= n;
        }
        GUI_DrawFilledRect(VX + 4, VY + vh - 44, vw - 8, 18, 0x44000000);
        GUI_DrawTextLine(VX + 8, VY + vh - 44, buf, 0xFFAACC88);
    }

    // Titulo da area
    char title[64];
    std::snprintf(title, sizeof(title), "%d points", g_pointCount);
    GUI_DrawTextLine(VX + 6, VY + 4, title, 0x88FFFFFF);
}

void DebugRandomTest_ApplyRendererState(float renderFPS, float frameMs)
{
    EnsureLayout();
    Renderer_SetOverlayText("InteractiveX r0.2", "Random test", g_statusLine);
    Renderer_SetHelpText(g_helpLine);
    Renderer_SetDisplayOptions(1, 1, 0);
    Renderer_SetControlButton(NULL);
    Renderer_SetControlButtons(g_buttons, 3);
    Renderer_SetControlSliders(g_sliders, 2);
    Renderer_SetOverlayMetrics(renderFPS, frameMs, 0.0f);
}

int  DebugRandomTest_ShouldReturnToMenu() { return g_returnToMenu; }
void DebugRandomTest_ClearReturnToMenu()  { g_returnToMenu = 0;    }
