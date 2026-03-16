#ifndef IX_CORE_SCENE_H
#define IX_CORE_SCENE_H

#include <windows.h>

#define IX_SCENE_MENU            0
#define IX_SCENE_FIRE_EFFECT     1
#define IX_SCENE_INPUT_TEST      2
#define IX_SCENE_TIMER_TEST      3
#define IX_SCENE_SCENE_TEST      4
#define IX_SCENE_AUDIO_TEST      5
#define IX_SCENE_ENTITY_TEST     6
#define IX_SCENE_CAMERA_TEST     7
#define IX_SCENE_IMAGE_TEST      8
#define IX_SCENE_TILEMAP_TEST    9
#define IX_SCENE_ANIM_TEST      10
#define IX_SCENE_PARTICLES_TEST 11
#define IX_SCENE_SAVELOAD_TEST  12
#define IX_SCENE_GAMEPAD_TEST   13
#define IX_SCENE_MATH2D_TEST     14
#define IX_SCENE_TRANSITION_TEST 15
#define IX_SCENE_RANDOM_TEST     16
#define IX_SCENE_VIDEO_TEST      17
#define IX_SCENE_USER_BASE      100

typedef struct IXSceneCallbacks
{
    void (*onEnter)           (void);
    void (*onExit)            (void);
    void (*onInvalidateLayout)(void);
    void (*update)            (int mouseX, int mouseY, int mouseDown, int mouseReleased);
    void (*render)            (void);
    void (*handleMouseMove)   (int x, int y);
    void (*handleDragBegin)   (int x, int y);
    void (*handleDragEnd)     (void);
    void (*handleKeyDown)     (WPARAM key);
} IXSceneCallbacks;

int  Scene_Register(int sceneId, IXSceneCallbacks callbacks);
void Scene_RegisterDebugScenes();
void Scene_SetCurrent(int sceneId);
int  Scene_GetCurrent();
void Scene_RequestChange(int sceneId);
int  Scene_HasPendingChange();
int  Scene_ConsumePendingChange();
void Scene_Initialize();
void Scene_OnEnterCurrent();
void Scene_InvalidateLayout();
void Scene_HandleMouseMove(int x, int y);
void Scene_BeginMouseDrag(int x, int y);
void Scene_EndMouseDrag();
void Scene_HandleKeyDown(WPARAM key);
void Scene_Update();
void Scene_Render();

#endif // IX_CORE_SCENE_H
