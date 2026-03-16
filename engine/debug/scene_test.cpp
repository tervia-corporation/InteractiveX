#include "scene_test.h"
#include "../renderer.h"
#include "../core/input.h"
#include "../interface/button.h"
#include "../interface/layout.h"
#include <cstdio>

static IXButton g_blueButton;
static IXButton g_redButton;
static IXButton g_greenButton;
static IXButton g_purpleButton;
static IXButton g_darkButton;
static const IXButton* g_buttons[5];

static int          g_returnToMenu    = 0;
static unsigned int g_backgroundColor = 0x18000000;
static char         g_statusLine[192] = "Scene test ready.";
static char         g_helpLine[160]   = "Pick a background color | ESC menu";

struct SceneLayoutCache { int valid, x, y, width, height; };
static SceneLayoutCache g_layoutCache = {0, 0, 0, 0, 0};

static void RefreshLayout()
{
    IXLayout layout;
    int x, y, width, height;
    Renderer_GetOverlayContentRect(&x, &y, &width, &height);
    g_layoutCache = { 1, x, y, width, height };

    Layout_BeginVertical(&layout, x, y + 88, width, height - 88, 12);
    Layout_Next(&layout, 38, &x, &y, &width, &height); Button_SetBounds(&g_blueButton,   x, y, width, height);
    Layout_Next(&layout, 38, &x, &y, &width, &height); Button_SetBounds(&g_redButton,    x, y, width, height);
    Layout_Next(&layout, 38, &x, &y, &width, &height); Button_SetBounds(&g_greenButton,  x, y, width, height);
    Layout_Next(&layout, 38, &x, &y, &width, &height); Button_SetBounds(&g_purpleButton, x, y, width, height);
    Layout_Next(&layout, 38, &x, &y, &width, &height); Button_SetBounds(&g_darkButton,   x, y, width, height);
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

static void SetTheme(unsigned int color, const char* name)
{
    g_backgroundColor = color;
    std::snprintf(g_statusLine, sizeof(g_statusLine),
        "Scene test active. Background: %s", name ? name : "Unknown");
}

void DebugSceneTest_Initialize()
{
    Button_Init(&g_blueButton,   28, 140, 220, 38, "Blue background");
    Button_Init(&g_redButton,    28, 190, 220, 38, "Red background");
    Button_Init(&g_greenButton,  28, 240, 220, 38, "Green background");
    Button_Init(&g_purpleButton, 28, 290, 220, 38, "Purple background");
    Button_Init(&g_darkButton,   28, 340, 220, 38, "Default dark");

    Button_SetColors(&g_redButton, 0xCC4A2222, 0xCC6A2F2F, 0xCC8A3A3A);
    Button_SetColors(&g_greenButton, 0xCC1E4030, 0xCC27553F, 0xCC2F6B4E);
    Button_SetColors(&g_purpleButton, 0xCC35264B, 0xCC4A3470, 0xCC63489A);
    Button_SetColors(&g_darkButton, 0xCC22252B, 0xCC303540, 0xCC3B4250);

    g_buttons[0] = &g_blueButton;
    g_buttons[1] = &g_redButton;
    g_buttons[2] = &g_greenButton;
    g_buttons[3] = &g_purpleButton;
    g_buttons[4] = &g_darkButton;

    g_returnToMenu      = 0;
    g_layoutCache.valid = 0;
    EnsureLayout();
    SetTheme(0x18000000, "Default dark");
}

void DebugSceneTest_InvalidateLayout()
{
    g_layoutCache.valid = 0;
}

void DebugSceneTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased)
{
    EnsureLayout();
    if (Input_WasKeyPressed(VK_ESCAPE))   g_returnToMenu = 1;
    if (Button_Handle(&g_blueButton,   mouseX, mouseY, mouseDown, mouseReleased)) SetTheme(0x18204060, "Blue");
    if (Button_Handle(&g_redButton,    mouseX, mouseY, mouseDown, mouseReleased)) SetTheme(0x184A1818, "Red");
    if (Button_Handle(&g_greenButton,  mouseX, mouseY, mouseDown, mouseReleased)) SetTheme(0x18183A24, "Green");
    if (Button_Handle(&g_purpleButton, mouseX, mouseY, mouseDown, mouseReleased)) SetTheme(0x18342056, "Purple");
    if (Button_Handle(&g_darkButton,   mouseX, mouseY, mouseDown, mouseReleased)) SetTheme(0x18000000, "Default dark");
}

void DebugSceneTest_ApplyRendererState(float renderFPS, float frameMs)
{
    EnsureLayout();
    Renderer_SetBackgroundColor(g_backgroundColor);
    Renderer_SetOverlayText("InteractiveX r0.2", "Scene debug", g_statusLine);
    Renderer_SetHelpText(g_helpLine);
    Renderer_SetDisplayOptions(1, 1, 1);
    Renderer_SetControlButton(NULL);
    Renderer_SetControlButtons(g_buttons, 5);
    Renderer_SetControlSliders(NULL, 0);
    Renderer_SetOverlayMetrics(renderFPS, frameMs, 0.0f);
}

int  DebugSceneTest_ShouldReturnToMenu() { return g_returnToMenu; }
void DebugSceneTest_ClearReturnToMenu()  { g_returnToMenu = 0;    }
