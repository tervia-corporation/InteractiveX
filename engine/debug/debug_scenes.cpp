#include "debug_scenes.h"
#include "../core/scene.h"
#include "../core/timer.h"
#include "../media/audio.h"
#include "../renderer.h"
#include "menu.h"
#include "fire_effect.h"
#include "input_test.h"
#include "timer_test.h"
#include "scene_test.h"
#include "audio_test.h"
#include "entity_test.h"
#include "camera_test.h"
#include "image_test.h"
#include "tilemap_test.h"
#include "anim_test.h"
#include "particles_test.h"
#include "saveload_test.h"
#include "gamepad_test.h"
#include "math2d_test.h"
#include "transition_test.h"
#include "random_test.h"
#include "video_test.h"

// ---------------------------------------------------------------------------
//  Menu
// ---------------------------------------------------------------------------
static void Menu_OnEnter() { MediaAudio_Stop(); DebugMenu_Initialize(); DebugMenu_ApplyRendererState(); }
static void Menu_InvalidateLayout() { DebugMenu_InvalidateLayout(); }
static void Menu_Update(int mx, int my, int md, int mr)
{
    DebugMenu_Update(mx, my, md, mr);
    IXDebugMenuAction a = DebugMenu_ConsumeAction();
    if      (a == IX_DEBUG_MENU_ACTION_OPEN_FIRE_EFFECT)    Scene_RequestChange(IX_SCENE_FIRE_EFFECT);
    else if (a == IX_DEBUG_MENU_ACTION_OPEN_INPUT_TEST)     Scene_RequestChange(IX_SCENE_INPUT_TEST);
    else if (a == IX_DEBUG_MENU_ACTION_OPEN_TIMER_TEST)     Scene_RequestChange(IX_SCENE_TIMER_TEST);
    else if (a == IX_DEBUG_MENU_ACTION_OPEN_SCENE_TEST)     Scene_RequestChange(IX_SCENE_SCENE_TEST);
    else if (a == IX_DEBUG_MENU_ACTION_OPEN_AUDIO_TEST)     Scene_RequestChange(IX_SCENE_AUDIO_TEST);
    else if (a == IX_DEBUG_MENU_ACTION_OPEN_ENTITY_TEST)    Scene_RequestChange(IX_SCENE_ENTITY_TEST);
    else if (a == IX_DEBUG_MENU_ACTION_OPEN_CAMERA_TEST)    Scene_RequestChange(IX_SCENE_CAMERA_TEST);
    else if (a == IX_DEBUG_MENU_ACTION_OPEN_IMAGE_TEST)     Scene_RequestChange(IX_SCENE_IMAGE_TEST);
    else if (a == IX_DEBUG_MENU_ACTION_OPEN_TILEMAP_TEST)   Scene_RequestChange(IX_SCENE_TILEMAP_TEST);
    else if (a == IX_DEBUG_MENU_ACTION_OPEN_ANIM_TEST)      Scene_RequestChange(IX_SCENE_ANIM_TEST);
    else if (a == IX_DEBUG_MENU_ACTION_OPEN_PARTICLES_TEST) Scene_RequestChange(IX_SCENE_PARTICLES_TEST);
    else if (a == IX_DEBUG_MENU_ACTION_OPEN_SAVELOAD_TEST)  Scene_RequestChange(IX_SCENE_SAVELOAD_TEST);
    else if (a == IX_DEBUG_MENU_ACTION_OPEN_GAMEPAD_TEST)   Scene_RequestChange(IX_SCENE_GAMEPAD_TEST);
    else if (a == IX_DEBUG_MENU_ACTION_OPEN_MATH2D_TEST)    Scene_RequestChange(IX_SCENE_MATH2D_TEST);
    else if (a == IX_DEBUG_MENU_ACTION_OPEN_TRANSITION_TEST) Scene_RequestChange(IX_SCENE_TRANSITION_TEST);
    else if (a == IX_DEBUG_MENU_ACTION_OPEN_RANDOM_TEST)     Scene_RequestChange(IX_SCENE_RANDOM_TEST);
    else if (a == IX_DEBUG_MENU_ACTION_OPEN_VIDEO_TEST)      Scene_RequestChange(IX_SCENE_VIDEO_TEST);
    else DebugMenu_ApplyRendererState();
}
static void Menu_Render() { DebugMenu_ApplyRendererState(); }

// ---------------------------------------------------------------------------
//  Fire Effect
// ---------------------------------------------------------------------------
static void Fire_OnEnter() { DebugFireEffect_Initialize(); DebugFireEffect_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds(), Timer_GetSimUPS()); }
static void Fire_InvalidateLayout() { DebugFireEffect_InvalidateLayout(); }
static void Fire_Update(int mx, int my, int md, int mr)
{
    unsigned int s = 0;
    DebugFireEffect_UpdateUI(mx, my, md, mr);
    DebugFireEffect_AccumulateSimulation(Timer_GetDeltaMilliseconds(), &s);
    Timer_AddSimulationSteps(s);
    if (DebugFireEffect_ShouldReturnToMenu()) { DebugFireEffect_ClearReturnToMenu(); Scene_RequestChange(IX_SCENE_MENU); }
}
static void Fire_Render()
{
    int vx, vy, vw, vh;
    DebugFireEffect_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds(), Timer_GetSimUPS());
    DebugFireEffect_GetViewport(&vx, &vy, &vw, &vh);
    Renderer_DrawPixelBuffer(DebugFireEffect_GetPixelBuffer(), DebugFireEffect_GetBufferWidth(), DebugFireEffect_GetBufferHeight(), vx, vy, vw, vh);
}
static void Fire_MouseMove(int x, int y) { DebugFireEffect_HandleMouseMove(x, y); }
static void Fire_DragBegin(int x, int y) { DebugFireEffect_BeginMouseDrag(x, y); }
static void Fire_DragEnd()               { DebugFireEffect_EndMouseDrag(); }
static void Fire_KeyDown(WPARAM key)     { DebugFireEffect_HandleKeyDown(key); }

// ---------------------------------------------------------------------------
//  Boilerplate compacto
// ---------------------------------------------------------------------------
static void InputTest_OnEnter()  { DebugInputTest_Initialize(); DebugInputTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); }
static void InputTest_Update(int mx, int my, int md, int mr) { (void)mx;(void)my;(void)md;(void)mr; DebugInputTest_Update(); if (DebugInputTest_ShouldReturnToMenu()) { DebugInputTest_ClearReturnToMenu(); Scene_RequestChange(IX_SCENE_MENU); } }
static void InputTest_Render()   { DebugInputTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); }

static void TimerTest_OnEnter()  { DebugTimerTest_Initialize(); DebugTimerTest_ApplyRendererState(); }
static void TimerTest_Update(int mx, int my, int md, int mr) { (void)mx;(void)my;(void)md;(void)mr; DebugTimerTest_Update(); if (DebugTimerTest_ShouldReturnToMenu()) { DebugTimerTest_ClearReturnToMenu(); Scene_RequestChange(IX_SCENE_MENU); } }
static void TimerTest_Render()   { DebugTimerTest_ApplyRendererState(); }

static void SceneTest_OnEnter()  { DebugSceneTest_Initialize(); DebugSceneTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); }
static void SceneTest_InvalidateLayout() { DebugSceneTest_InvalidateLayout(); }
static void SceneTest_Update(int mx, int my, int md, int mr) { DebugSceneTest_Update(mx, my, md, mr); if (DebugSceneTest_ShouldReturnToMenu()) { DebugSceneTest_ClearReturnToMenu(); Scene_RequestChange(IX_SCENE_MENU); } }
static void SceneTest_Render()   { DebugSceneTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); }

static void AudioTest_OnEnter()  { DebugAudioTest_Initialize(); DebugAudioTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); }
static void AudioTest_InvalidateLayout() { DebugAudioTest_InvalidateLayout(); }
static void AudioTest_Update(int mx, int my, int md, int mr) { DebugAudioTest_Update(mx, my, md, mr); if (DebugAudioTest_ShouldReturnToMenu()) { DebugAudioTest_ClearReturnToMenu(); Scene_RequestChange(IX_SCENE_MENU); } }
static void AudioTest_Render()   { DebugAudioTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); }

static void EntityTest_OnEnter() { DebugEntityTest_Initialize(); DebugEntityTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); }
static void EntityTest_InvalidateLayout() { DebugEntityTest_InvalidateLayout(); }
static void EntityTest_Update(int mx, int my, int md, int mr) { DebugEntityTest_Update(mx, my, md, mr); if (DebugEntityTest_ShouldReturnToMenu()) { DebugEntityTest_ClearReturnToMenu(); Scene_RequestChange(IX_SCENE_MENU); } }
static void EntityTest_Render()  { DebugEntityTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); DebugEntityTest_Render(); }
static void EntityTest_KeyDown(WPARAM key) { DebugEntityTest_HandleKeyDown((unsigned int)key); }

static void CameraTest_OnEnter() { DebugCameraTest_Initialize(); DebugCameraTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); }
static void CameraTest_InvalidateLayout() { DebugCameraTest_InvalidateLayout(); }
static void CameraTest_Update(int mx, int my, int md, int mr) { DebugCameraTest_Update(mx, my, md, mr); if (DebugCameraTest_ShouldReturnToMenu()) { DebugCameraTest_ClearReturnToMenu(); Scene_RequestChange(IX_SCENE_MENU); } }
static void CameraTest_Render()  { DebugCameraTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); DebugCameraTest_Render(); }
static void CameraTest_KeyDown(WPARAM key) { DebugCameraTest_HandleKeyDown((unsigned int)key); }

static void ImageTest_OnEnter()  { DebugImageTest_Initialize(); DebugImageTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); }
static void ImageTest_Update(int mx, int my, int md, int mr) { DebugImageTest_Update(mx, my, md, mr); if (DebugImageTest_ShouldReturnToMenu()) { DebugImageTest_ClearReturnToMenu(); Scene_RequestChange(IX_SCENE_MENU); } }
static void ImageTest_Render()   { DebugImageTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); DebugImageTest_Render(); }

static void TilemapTest_OnEnter() { DebugTilemapTest_Initialize(); DebugTilemapTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); }
static void TilemapTest_InvalidateLayout() { DebugTilemapTest_InvalidateLayout(); }
static void TilemapTest_Update(int mx, int my, int md, int mr) { DebugTilemapTest_Update(mx, my, md, mr); if (DebugTilemapTest_ShouldReturnToMenu()) { DebugTilemapTest_ClearReturnToMenu(); Scene_RequestChange(IX_SCENE_MENU); } }
static void TilemapTest_Render() { DebugTilemapTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); DebugTilemapTest_Render(); }
static void TilemapTest_KeyDown(WPARAM key) { DebugTilemapTest_HandleKeyDown((unsigned int)key); }

static void AnimTest_OnEnter()   { DebugAnimTest_Initialize(); DebugAnimTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); }
static void AnimTest_Update(int mx, int my, int md, int mr) { DebugAnimTest_Update(mx, my, md, mr); if (DebugAnimTest_ShouldReturnToMenu()) { DebugAnimTest_ClearReturnToMenu(); Scene_RequestChange(IX_SCENE_MENU); } }
static void AnimTest_Render()    { DebugAnimTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); DebugAnimTest_Render(); }
static void AnimTest_KeyDown(WPARAM key) { DebugAnimTest_HandleKeyDown((unsigned int)key); }

// ---------------------------------------------------------------------------
//  Particles Test
// ---------------------------------------------------------------------------
static void ParticlesTest_OnEnter()  { DebugParticlesTest_Initialize(); DebugParticlesTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); }
static void ParticlesTest_Update(int mx, int my, int md, int mr) { DebugParticlesTest_Update(mx, my, md, mr); if (DebugParticlesTest_ShouldReturnToMenu()) { DebugParticlesTest_ClearReturnToMenu(); Scene_RequestChange(IX_SCENE_MENU); } }
static void ParticlesTest_Render()   { DebugParticlesTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); DebugParticlesTest_Render(); }
static void ParticlesTest_KeyDown(WPARAM key) { DebugParticlesTest_HandleKeyDown((unsigned int)key); }

// ---------------------------------------------------------------------------
//  Save/Load Test
// ---------------------------------------------------------------------------
static void SaveLoadTest_OnEnter()  { DebugSaveLoadTest_Initialize(); DebugSaveLoadTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); }
static void SaveLoadTest_Update(int mx, int my, int md, int mr) { DebugSaveLoadTest_Update(mx, my, md, mr); if (DebugSaveLoadTest_ShouldReturnToMenu()) { DebugSaveLoadTest_ClearReturnToMenu(); Scene_RequestChange(IX_SCENE_MENU); } }
static void SaveLoadTest_Render()   { DebugSaveLoadTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); }

// ---------------------------------------------------------------------------
//  Gamepad Test
// ---------------------------------------------------------------------------
static void GamepadTest_OnEnter()  { DebugGamepadTest_Initialize(); DebugGamepadTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); }
static void GamepadTest_Update(int mx, int my, int md, int mr) { DebugGamepadTest_Update(mx, my, md, mr); if (DebugGamepadTest_ShouldReturnToMenu()) { DebugGamepadTest_ClearReturnToMenu(); Scene_RequestChange(IX_SCENE_MENU); } }
static void GamepadTest_Render()   { DebugGamepadTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); DebugGamepadTest_Render(); }

static void Math2DTest_OnEnter();
static void Math2DTest_Update(int mx, int my, int md, int mr);
static void Math2DTest_Render();
static void TransitionTest_OnEnter();
static void TransitionTest_InvalidateLayout();
static void TransitionTest_Update(int mx, int my, int md, int mr);
static void TransitionTest_Render();
static void RandomTest_OnEnter();
static void RandomTest_InvalidateLayout();
static void RandomTest_Update(int mx, int my, int md, int mr);
static void RandomTest_Render();
static void VideoTest_OnEnter();
static void VideoTest_InvalidateLayout();
static void VideoTest_Update(int mx, int my, int md, int mr);
static void VideoTest_Render();

// ---------------------------------------------------------------------------
//  Registration
// ---------------------------------------------------------------------------
void Scene_RegisterDebugScenes()
{
    { IXSceneCallbacks cb = { Menu_OnEnter, NULL, Menu_InvalidateLayout, Menu_Update, Menu_Render, NULL, NULL, NULL, NULL }; Scene_Register(IX_SCENE_MENU, cb); }
    { IXSceneCallbacks cb = { Fire_OnEnter, NULL, Fire_InvalidateLayout, Fire_Update, Fire_Render, Fire_MouseMove, Fire_DragBegin, Fire_DragEnd, Fire_KeyDown }; Scene_Register(IX_SCENE_FIRE_EFFECT, cb); }
    { IXSceneCallbacks cb = { InputTest_OnEnter, NULL, NULL, InputTest_Update, InputTest_Render, NULL, NULL, NULL, NULL }; Scene_Register(IX_SCENE_INPUT_TEST, cb); }
    { IXSceneCallbacks cb = { TimerTest_OnEnter, NULL, NULL, TimerTest_Update, TimerTest_Render, NULL, NULL, NULL, NULL }; Scene_Register(IX_SCENE_TIMER_TEST, cb); }
    { IXSceneCallbacks cb = { SceneTest_OnEnter, NULL, SceneTest_InvalidateLayout, SceneTest_Update, SceneTest_Render, NULL, NULL, NULL, NULL }; Scene_Register(IX_SCENE_SCENE_TEST, cb); }
    { IXSceneCallbacks cb = { AudioTest_OnEnter, NULL, AudioTest_InvalidateLayout, AudioTest_Update, AudioTest_Render, NULL, NULL, NULL, NULL }; Scene_Register(IX_SCENE_AUDIO_TEST, cb); }
    { IXSceneCallbacks cb = { EntityTest_OnEnter, NULL, EntityTest_InvalidateLayout, EntityTest_Update, EntityTest_Render, NULL, NULL, NULL, EntityTest_KeyDown }; Scene_Register(IX_SCENE_ENTITY_TEST, cb); }
    { IXSceneCallbacks cb = { CameraTest_OnEnter, NULL, CameraTest_InvalidateLayout, CameraTest_Update, CameraTest_Render, NULL, NULL, NULL, CameraTest_KeyDown }; Scene_Register(IX_SCENE_CAMERA_TEST, cb); }
    { IXSceneCallbacks cb = { ImageTest_OnEnter, NULL, NULL, ImageTest_Update, ImageTest_Render, NULL, NULL, NULL, NULL }; Scene_Register(IX_SCENE_IMAGE_TEST, cb); }
    { IXSceneCallbacks cb = { TilemapTest_OnEnter, NULL, TilemapTest_InvalidateLayout, TilemapTest_Update, TilemapTest_Render, NULL, NULL, NULL, TilemapTest_KeyDown }; Scene_Register(IX_SCENE_TILEMAP_TEST, cb); }
    { IXSceneCallbacks cb = { AnimTest_OnEnter, NULL, NULL, AnimTest_Update, AnimTest_Render, NULL, NULL, NULL, AnimTest_KeyDown }; Scene_Register(IX_SCENE_ANIM_TEST, cb); }
    { IXSceneCallbacks cb = { ParticlesTest_OnEnter, NULL, NULL, ParticlesTest_Update, ParticlesTest_Render, NULL, NULL, NULL, ParticlesTest_KeyDown }; Scene_Register(IX_SCENE_PARTICLES_TEST, cb); }
    { IXSceneCallbacks cb = { SaveLoadTest_OnEnter, NULL, NULL, SaveLoadTest_Update, SaveLoadTest_Render, NULL, NULL, NULL, NULL }; Scene_Register(IX_SCENE_SAVELOAD_TEST, cb); }
    { IXSceneCallbacks cb = { GamepadTest_OnEnter, NULL, NULL, GamepadTest_Update, GamepadTest_Render, NULL, NULL, NULL, NULL }; Scene_Register(IX_SCENE_GAMEPAD_TEST, cb); }
    { IXSceneCallbacks cb = { Math2DTest_OnEnter, NULL, NULL, Math2DTest_Update, Math2DTest_Render, NULL, NULL, NULL, NULL }; Scene_Register(IX_SCENE_MATH2D_TEST, cb); }
    { IXSceneCallbacks cb = { TransitionTest_OnEnter, NULL, TransitionTest_InvalidateLayout, TransitionTest_Update, TransitionTest_Render, NULL, NULL, NULL, NULL }; Scene_Register(IX_SCENE_TRANSITION_TEST, cb); }
    { IXSceneCallbacks cb = { RandomTest_OnEnter, NULL, RandomTest_InvalidateLayout, RandomTest_Update, RandomTest_Render, NULL, NULL, NULL, NULL }; Scene_Register(IX_SCENE_RANDOM_TEST, cb); }
    { IXSceneCallbacks cb = { VideoTest_OnEnter, NULL, VideoTest_InvalidateLayout, VideoTest_Update, VideoTest_Render, NULL, NULL, NULL, NULL }; Scene_Register(IX_SCENE_VIDEO_TEST, cb); }
}

// Math2D inserido por patch — callbacks
static void Math2DTest_OnEnter()  { DebugMath2DTest_Initialize(); DebugMath2DTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); }
static void Math2DTest_Update(int mx, int my, int md, int mr) { DebugMath2DTest_Update(mx, my, md, mr); if (DebugMath2DTest_ShouldReturnToMenu()) { DebugMath2DTest_ClearReturnToMenu(); Scene_RequestChange(IX_SCENE_MENU); } }
static void Math2DTest_Render()   { DebugMath2DTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); DebugMath2DTest_Render(); }

// Transition Test
static void TransitionTest_OnEnter()  { DebugTransitionTest_Initialize(); DebugTransitionTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); }
static void TransitionTest_InvalidateLayout() { DebugTransitionTest_InvalidateLayout(); }
static void TransitionTest_Update(int mx, int my, int md, int mr) { DebugTransitionTest_Update(mx, my, md, mr); if (DebugTransitionTest_ShouldReturnToMenu()) { DebugTransitionTest_ClearReturnToMenu(); Scene_RequestChange(IX_SCENE_MENU); } }
static void TransitionTest_Render()   { DebugTransitionTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); }

// Random Test
static void RandomTest_OnEnter()  { DebugRandomTest_Initialize(); DebugRandomTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); }
static void RandomTest_InvalidateLayout() { DebugRandomTest_InvalidateLayout(); }
static void RandomTest_Update(int mx, int my, int md, int mr) { DebugRandomTest_Update(mx, my, md, mr); if (DebugRandomTest_ShouldReturnToMenu()) { DebugRandomTest_ClearReturnToMenu(); Scene_RequestChange(IX_SCENE_MENU); } }
static void RandomTest_Render()   { DebugRandomTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); DebugRandomTest_Render(); }

// Video Test
static void VideoTest_OnEnter()  { DebugVideoTest_Initialize(); DebugVideoTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); }
static void VideoTest_InvalidateLayout() { DebugVideoTest_InvalidateLayout(); }
static void VideoTest_Update(int mx, int my, int md, int mr) { DebugVideoTest_Update(mx, my, md, mr); if (DebugVideoTest_ShouldReturnToMenu()) { DebugVideoTest_ClearReturnToMenu(); Scene_RequestChange(IX_SCENE_MENU); } }
static void VideoTest_Render()   { DebugVideoTest_ApplyRendererState(Timer_GetRenderFPS(), Timer_GetSmoothedFrameMilliseconds()); DebugVideoTest_Render(); }
