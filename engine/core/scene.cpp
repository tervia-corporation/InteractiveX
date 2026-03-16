#include "scene.h"
#include "input.h"
#include "timer.h"
#include "../renderer.h"

// ---------------------------------------------------------------------------
//  Scene registry
// ---------------------------------------------------------------------------
#define IX_MAX_SCENES 64

static IXSceneCallbacks g_callbacks[IX_MAX_SCENES];
static int              g_ids[IX_MAX_SCENES];
static int              g_count = 0;

static IXSceneCallbacks* FindScene(int id)
{
    for (int i = 0; i < g_count; i++)
        if (g_ids[i] == id) return &g_callbacks[i];
    return NULL;
}

// ---------------------------------------------------------------------------
//  Navigation state
// ---------------------------------------------------------------------------
static int g_current    = IX_SCENE_MENU;
static int g_pending    = IX_SCENE_MENU;
static int g_hasPending = 0;

// ---------------------------------------------------------------------------
//  Registration
// ---------------------------------------------------------------------------
void Scene_Initialize()
{
    g_count      = 0;
    g_current    = IX_SCENE_MENU;
    g_pending    = IX_SCENE_MENU;
    g_hasPending = 0;
}

int Scene_Register(int sceneId, IXSceneCallbacks callbacks)
{
    for (int i = 0; i < g_count; i++)
    {
        if (g_ids[i] == sceneId)
        {
            g_callbacks[i] = callbacks;
            return 1;
        }
    }
    if (g_count >= IX_MAX_SCENES) return 0;
    g_ids[g_count]       = sceneId;
    g_callbacks[g_count] = callbacks;
    g_count++;
    return 1;
}

// ---------------------------------------------------------------------------
//  Navigation
// ---------------------------------------------------------------------------
void Scene_SetCurrent(int id)  { g_current = id; g_hasPending = 0; }
int  Scene_GetCurrent()        { return g_current; }
void Scene_RequestChange(int id) { g_pending = id; g_hasPending = 1; }
int  Scene_HasPendingChange()  { return g_hasPending; }
int  Scene_ConsumePendingChange() { g_hasPending = 0; return g_pending; }

// ---------------------------------------------------------------------------
//  Engine internal callbacks
// ---------------------------------------------------------------------------
void Scene_OnEnterCurrent()
{
    Renderer_SetBackgroundColor(0x18000000);
    IXSceneCallbacks* cb = FindScene(g_current);
    if (cb && cb->onEnter) cb->onEnter();
}

void Scene_InvalidateLayout()
{
    IXSceneCallbacks* cb = FindScene(g_current);
    if (cb && cb->onInvalidateLayout) cb->onInvalidateLayout();
}

void Scene_HandleMouseMove(int x, int y)
{
    IXSceneCallbacks* cb = FindScene(g_current);
    if (cb && cb->handleMouseMove) cb->handleMouseMove(x, y);
}

void Scene_BeginMouseDrag(int x, int y)
{
    IXSceneCallbacks* cb = FindScene(g_current);
    if (cb && cb->handleDragBegin) cb->handleDragBegin(x, y);
}

void Scene_EndMouseDrag()
{
    IXSceneCallbacks* cb = FindScene(g_current);
    if (cb && cb->handleDragEnd) cb->handleDragEnd();
}

void Scene_HandleKeyDown(WPARAM key)
{
    IXSceneCallbacks* cb = FindScene(g_current);
    if (cb && cb->handleKeyDown) cb->handleKeyDown(key);
}

void Scene_Update()
{
    int mouseX        = Input_GetMouseX();
    int mouseY        = Input_GetMouseY();
    int mouseDown     = Input_IsMouseButtonDown(IX_MOUSE_LEFT);
    int mouseReleased = Input_WasMouseButtonReleased(IX_MOUSE_LEFT);

    IXSceneCallbacks* cb = FindScene(g_current);
    if (cb && cb->update) cb->update(mouseX, mouseY, mouseDown, mouseReleased);
}

void Scene_Render()
{
    IXSceneCallbacks* cb = FindScene(g_current);
    Renderer_BeginFrame();
    if (cb && cb->render) cb->render();
    Renderer_EndFrame();
}
