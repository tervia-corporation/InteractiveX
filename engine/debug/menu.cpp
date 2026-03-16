#include "menu.h"
#include "../renderer.h"
#include "../interface/layout.h"
#include <cstdio>

#define MENU_BUTTON_COUNT 17

static IXButton g_fireEffectButton;
static IXButton g_inputTestButton;
static IXButton g_timerTestButton;
static IXButton g_sceneTestButton;
static IXButton g_audioTestButton;
static IXButton g_entityTestButton;
static IXButton g_cameraTestButton;
static IXButton g_imageTestButton;
static IXButton g_tilemapTestButton;
static IXButton g_animTestButton;
static IXButton g_particlesTestButton;
static IXButton g_saveLoadTestButton;
static IXButton g_gamepadTestButton;
static IXButton g_math2dTestButton;
static IXButton g_transitionTestButton;
static IXButton g_randomTestButton;
static IXButton g_videoTestButton;
static const IXButton* g_menuButtons[MENU_BUTTON_COUNT];

static IXDebugMenuAction g_pendingAction = IX_DEBUG_MENU_ACTION_NONE;
static char g_statusLine[160] = "Select a test module.";

struct MenuLayoutCache { int valid, x, y, width, height; };
static MenuLayoutCache g_layoutCache = {0, 0, 0, 0, 0};

static void RefreshMenuLayout()
{
    int x, y, width, height;
    Renderer_GetOverlayButtonsRect(&x, &y, &width, &height);
    g_layoutCache = { 1, x, y, width, height };

    const int SPACING = 4;
    const int N       = MENU_BUTTON_COUNT;
    int btnH = (height - SPACING * (N - 1)) / N;
    if (btnH < 20) btnH = 20;
    if (btnH > 36) btnH = 36;

    IXLayout layout;
    Layout_BeginVertical(&layout, x, y, width, height, SPACING);
    Layout_Next(&layout, btnH, &x, &y, &width, &height); Button_SetBounds(&g_fireEffectButton,    x, y, width, height);
    Layout_Next(&layout, btnH, &x, &y, &width, &height); Button_SetBounds(&g_inputTestButton,     x, y, width, height);
    Layout_Next(&layout, btnH, &x, &y, &width, &height); Button_SetBounds(&g_timerTestButton,     x, y, width, height);
    Layout_Next(&layout, btnH, &x, &y, &width, &height); Button_SetBounds(&g_sceneTestButton,     x, y, width, height);
    Layout_Next(&layout, btnH, &x, &y, &width, &height); Button_SetBounds(&g_audioTestButton,     x, y, width, height);
    Layout_Next(&layout, btnH, &x, &y, &width, &height); Button_SetBounds(&g_entityTestButton,    x, y, width, height);
    Layout_Next(&layout, btnH, &x, &y, &width, &height); Button_SetBounds(&g_cameraTestButton,    x, y, width, height);
    Layout_Next(&layout, btnH, &x, &y, &width, &height); Button_SetBounds(&g_imageTestButton,     x, y, width, height);
    Layout_Next(&layout, btnH, &x, &y, &width, &height); Button_SetBounds(&g_tilemapTestButton,   x, y, width, height);
    Layout_Next(&layout, btnH, &x, &y, &width, &height); Button_SetBounds(&g_animTestButton,      x, y, width, height);
    Layout_Next(&layout, btnH, &x, &y, &width, &height); Button_SetBounds(&g_particlesTestButton, x, y, width, height);
    Layout_Next(&layout, btnH, &x, &y, &width, &height); Button_SetBounds(&g_saveLoadTestButton,  x, y, width, height);
    Layout_Next(&layout, btnH, &x, &y, &width, &height); Button_SetBounds(&g_gamepadTestButton,   x, y, width, height);
    Layout_Next(&layout, btnH, &x, &y, &width, &height); Button_SetBounds(&g_math2dTestButton,      x, y, width, height);
    Layout_Next(&layout, btnH, &x, &y, &width, &height); Button_SetBounds(&g_transitionTestButton,  x, y, width, height);
    Layout_Next(&layout, btnH, &x, &y, &width, &height); Button_SetBounds(&g_randomTestButton,      x, y, width, height);
    Layout_Next(&layout, btnH, &x, &y, &width, &height); Button_SetBounds(&g_videoTestButton,       x, y, width, height);
}

static void EnsureMenuLayout()
{
    int x, y, w, h;
    Renderer_GetOverlayButtonsRect(&x, &y, &w, &h);
    if (!g_layoutCache.valid ||
        g_layoutCache.x != x || g_layoutCache.y != y ||
        g_layoutCache.width != w || g_layoutCache.height != h)
        RefreshMenuLayout();
}

void DebugMenu_Initialize()
{
    Button_Init(&g_fireEffectButton,    0, 0, 0, 0, "Fire Effect");
    Button_Init(&g_inputTestButton,     0, 0, 0, 0, "Input Test");
    Button_Init(&g_timerTestButton,     0, 0, 0, 0, "Timer Test");
    Button_Init(&g_sceneTestButton,     0, 0, 0, 0, "Scene Test");
    Button_Init(&g_audioTestButton,     0, 0, 0, 0, "Audio Test");
    Button_Init(&g_entityTestButton,    0, 0, 0, 0, "Entity Test");
    Button_Init(&g_cameraTestButton,    0, 0, 0, 0, "Camera Test");
    Button_Init(&g_imageTestButton,     0, 0, 0, 0, "Image Test");
    Button_Init(&g_tilemapTestButton,   0, 0, 0, 0, "Tilemap Test");
    Button_Init(&g_animTestButton,      0, 0, 0, 0, "Animation Test");
    Button_Init(&g_particlesTestButton, 0, 0, 0, 0, "Particles Test");
    Button_Init(&g_saveLoadTestButton,  0, 0, 0, 0, "Save/Load Test");
    Button_Init(&g_gamepadTestButton,   0, 0, 0, 0, "Gamepad Test");
    Button_Init(&g_math2dTestButton,      0, 0, 0, 0, "Math2D Test");
    Button_Init(&g_transitionTestButton,  0, 0, 0, 0, "Transition Test");
    Button_Init(&g_randomTestButton,      0, 0, 0, 0, "Random Test");
    Button_Init(&g_videoTestButton,       0, 0, 0, 0, "Video Test");

    Button_SetColors(&g_inputTestButton, 0xCC20324A, 0xCC294365, 0xCC35608F);
    Button_SetColors(&g_timerTestButton, 0xCC3A2A4A, 0xCC4E3870, 0xCC6A4B96);
    Button_SetColors(&g_sceneTestButton, 0xCC2F324A, 0xCC3F4668, 0xCC566091);
    Button_SetColors(&g_audioTestButton, 0xCC4A3020, 0xCC68432B, 0xCC8A5938);
    Button_SetColors(&g_entityTestButton, 0xCC1E3A28, 0xCC285038, 0xCC346848);
    Button_SetColors(&g_cameraTestButton, 0xCC1A3040, 0xCC224060, 0xCC2C5580);
    Button_SetColors(&g_imageTestButton, 0xCC3A2A40, 0xCC503858, 0xCC6A4870);
    Button_SetColors(&g_tilemapTestButton, 0xCC283820, 0xCC385030, 0xCC4A6840);
    Button_SetColors(&g_animTestButton, 0xCC403018, 0xCC584020, 0xCC785530);
    Button_SetColors(&g_particlesTestButton, 0xCC401818, 0xCC602020, 0xCC882828);
    Button_SetColors(&g_saveLoadTestButton, 0xCC1C3830, 0xCC264E42, 0xCC306858);
    Button_SetColors(&g_gamepadTestButton, 0xCC303018, 0xCC484820, 0xCC606030);
    Button_SetColors(&g_math2dTestButton, 0xCC183040, 0xCC224460, 0xCC2C5880);
    Button_SetColors(&g_transitionTestButton, 0xCC301830, 0xCC482248, 0xCC602E60);
    Button_SetColors(&g_randomTestButton, 0xCC2A2010, 0xCC3E3018, 0xCC544220);

    g_menuButtons[0]  = &g_fireEffectButton;
    g_menuButtons[1]  = &g_inputTestButton;
    g_menuButtons[2]  = &g_timerTestButton;
    g_menuButtons[3]  = &g_sceneTestButton;
    g_menuButtons[4]  = &g_audioTestButton;
    g_menuButtons[5]  = &g_entityTestButton;
    g_menuButtons[6]  = &g_cameraTestButton;
    g_menuButtons[7]  = &g_imageTestButton;
    g_menuButtons[8]  = &g_tilemapTestButton;
    g_menuButtons[9]  = &g_animTestButton;
    g_menuButtons[10] = &g_particlesTestButton;
    g_menuButtons[11] = &g_saveLoadTestButton;
    g_menuButtons[12] = &g_gamepadTestButton;
    g_menuButtons[13] = &g_math2dTestButton;
    g_menuButtons[14] = &g_transitionTestButton;
    g_menuButtons[15] = &g_randomTestButton;
    g_menuButtons[16] = &g_videoTestButton;

    g_pendingAction     = IX_DEBUG_MENU_ACTION_NONE;
    g_layoutCache.valid = 0;
    std::snprintf(g_statusLine, sizeof(g_statusLine), "Select a test module.");
    EnsureMenuLayout();
}

void DebugMenu_InvalidateLayout() { g_layoutCache.valid = 0; }

void DebugMenu_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased)
{
    EnsureMenuLayout();
    if (Button_Handle(&g_fireEffectButton,    mouseX, mouseY, mouseDown, mouseReleased)) { g_pendingAction = IX_DEBUG_MENU_ACTION_OPEN_FIRE_EFFECT;    std::snprintf(g_statusLine, sizeof(g_statusLine), "Opening Fire Effect..."); }
    if (Button_Handle(&g_inputTestButton,     mouseX, mouseY, mouseDown, mouseReleased)) { g_pendingAction = IX_DEBUG_MENU_ACTION_OPEN_INPUT_TEST;     std::snprintf(g_statusLine, sizeof(g_statusLine), "Opening Input Test..."); }
    if (Button_Handle(&g_timerTestButton,     mouseX, mouseY, mouseDown, mouseReleased)) { g_pendingAction = IX_DEBUG_MENU_ACTION_OPEN_TIMER_TEST;     std::snprintf(g_statusLine, sizeof(g_statusLine), "Opening Timer Test..."); }
    if (Button_Handle(&g_sceneTestButton,     mouseX, mouseY, mouseDown, mouseReleased)) { g_pendingAction = IX_DEBUG_MENU_ACTION_OPEN_SCENE_TEST;     std::snprintf(g_statusLine, sizeof(g_statusLine), "Opening Scene Test..."); }
    if (Button_Handle(&g_audioTestButton,     mouseX, mouseY, mouseDown, mouseReleased)) { g_pendingAction = IX_DEBUG_MENU_ACTION_OPEN_AUDIO_TEST;     std::snprintf(g_statusLine, sizeof(g_statusLine), "Opening Audio Test..."); }
    if (Button_Handle(&g_entityTestButton,    mouseX, mouseY, mouseDown, mouseReleased)) { g_pendingAction = IX_DEBUG_MENU_ACTION_OPEN_ENTITY_TEST;    std::snprintf(g_statusLine, sizeof(g_statusLine), "Opening Entity Test..."); }
    if (Button_Handle(&g_cameraTestButton,    mouseX, mouseY, mouseDown, mouseReleased)) { g_pendingAction = IX_DEBUG_MENU_ACTION_OPEN_CAMERA_TEST;    std::snprintf(g_statusLine, sizeof(g_statusLine), "Opening Camera Test..."); }
    if (Button_Handle(&g_imageTestButton,     mouseX, mouseY, mouseDown, mouseReleased)) { g_pendingAction = IX_DEBUG_MENU_ACTION_OPEN_IMAGE_TEST;     std::snprintf(g_statusLine, sizeof(g_statusLine), "Opening Image Test..."); }
    if (Button_Handle(&g_tilemapTestButton,   mouseX, mouseY, mouseDown, mouseReleased)) { g_pendingAction = IX_DEBUG_MENU_ACTION_OPEN_TILEMAP_TEST;   std::snprintf(g_statusLine, sizeof(g_statusLine), "Opening Tilemap Test..."); }
    if (Button_Handle(&g_animTestButton,      mouseX, mouseY, mouseDown, mouseReleased)) { g_pendingAction = IX_DEBUG_MENU_ACTION_OPEN_ANIM_TEST;      std::snprintf(g_statusLine, sizeof(g_statusLine), "Opening Animation Test..."); }
    if (Button_Handle(&g_particlesTestButton, mouseX, mouseY, mouseDown, mouseReleased)) { g_pendingAction = IX_DEBUG_MENU_ACTION_OPEN_PARTICLES_TEST; std::snprintf(g_statusLine, sizeof(g_statusLine), "Opening Particles Test..."); }
    if (Button_Handle(&g_saveLoadTestButton,  mouseX, mouseY, mouseDown, mouseReleased)) { g_pendingAction = IX_DEBUG_MENU_ACTION_OPEN_SAVELOAD_TEST;  std::snprintf(g_statusLine, sizeof(g_statusLine), "Opening Save/Load Test..."); }
    if (Button_Handle(&g_gamepadTestButton,   mouseX, mouseY, mouseDown, mouseReleased)) { g_pendingAction = IX_DEBUG_MENU_ACTION_OPEN_GAMEPAD_TEST;   std::snprintf(g_statusLine, sizeof(g_statusLine), "Opening Gamepad Test..."); }
    if (Button_Handle(&g_math2dTestButton,      mouseX, mouseY, mouseDown, mouseReleased)) { g_pendingAction = IX_DEBUG_MENU_ACTION_OPEN_MATH2D_TEST;      std::snprintf(g_statusLine, sizeof(g_statusLine), "Opening Math2D Test..."); }
    if (Button_Handle(&g_transitionTestButton,  mouseX, mouseY, mouseDown, mouseReleased)) { g_pendingAction = IX_DEBUG_MENU_ACTION_OPEN_TRANSITION_TEST;  std::snprintf(g_statusLine, sizeof(g_statusLine), "Opening Transition Test..."); }
    if (Button_Handle(&g_randomTestButton,      mouseX, mouseY, mouseDown, mouseReleased)) { g_pendingAction = IX_DEBUG_MENU_ACTION_OPEN_RANDOM_TEST;      std::snprintf(g_statusLine, sizeof(g_statusLine), "Opening Random Test..."); }
    if (Button_Handle(&g_videoTestButton,       mouseX, mouseY, mouseDown, mouseReleased)) { g_pendingAction = IX_DEBUG_MENU_ACTION_OPEN_VIDEO_TEST;       std::snprintf(g_statusLine, sizeof(g_statusLine), "Opening Video Test..."); }
}

void DebugMenu_ApplyRendererState()
{
    EnsureMenuLayout();
    Renderer_SetOverlayText("InteractiveX", "r0.2", g_statusLine);
    Renderer_SetHelpText(NULL);
    Renderer_SetDisplayOptions(0, 0, 1);
    Renderer_SetControlButton(NULL);
    Renderer_SetControlButtons(g_menuButtons, MENU_BUTTON_COUNT);
    Renderer_SetControlSliders(NULL, 0);
}

int DebugMenu_IsPointOverUI(int x, int y)
{
    EnsureMenuLayout();
    for (int i = 0; i < MENU_BUTTON_COUNT; i++)
        if (Button_IsInside(g_menuButtons[i], x, y)) return 1;
    return 0;
}

IXDebugMenuAction DebugMenu_ConsumeAction()
{
    IXDebugMenuAction action = g_pendingAction;
    g_pendingAction = IX_DEBUG_MENU_ACTION_NONE;
    return action;
}
