#ifndef IX_DEBUG_MENU_H
#define IX_DEBUG_MENU_H

#include "../interface/button.h"

enum IXDebugMenuAction
{
    IX_DEBUG_MENU_ACTION_NONE                  = 0,
    IX_DEBUG_MENU_ACTION_OPEN_FIRE_EFFECT      = 1,
    IX_DEBUG_MENU_ACTION_OPEN_INPUT_TEST       = 2,
    IX_DEBUG_MENU_ACTION_OPEN_TIMER_TEST       = 3,
    IX_DEBUG_MENU_ACTION_OPEN_SCENE_TEST       = 4,
    IX_DEBUG_MENU_ACTION_OPEN_AUDIO_TEST       = 5,
    IX_DEBUG_MENU_ACTION_OPEN_ENTITY_TEST      = 6,
    IX_DEBUG_MENU_ACTION_OPEN_CAMERA_TEST      = 7,
    IX_DEBUG_MENU_ACTION_OPEN_IMAGE_TEST       = 8,
    IX_DEBUG_MENU_ACTION_OPEN_TILEMAP_TEST     = 9,
    IX_DEBUG_MENU_ACTION_OPEN_ANIM_TEST        = 10,
    IX_DEBUG_MENU_ACTION_OPEN_PARTICLES_TEST   = 11,
    IX_DEBUG_MENU_ACTION_OPEN_SAVELOAD_TEST    = 12,
    IX_DEBUG_MENU_ACTION_OPEN_GAMEPAD_TEST     = 13,
    IX_DEBUG_MENU_ACTION_OPEN_MATH2D_TEST      = 14,
    IX_DEBUG_MENU_ACTION_OPEN_TRANSITION_TEST  = 15,
    IX_DEBUG_MENU_ACTION_OPEN_RANDOM_TEST      = 16,
    IX_DEBUG_MENU_ACTION_OPEN_VIDEO_TEST       = 17
};

void              DebugMenu_Initialize();
void              DebugMenu_InvalidateLayout();
void              DebugMenu_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased);
void              DebugMenu_ApplyRendererState();
int               DebugMenu_IsPointOverUI(int x, int y);
IXDebugMenuAction DebugMenu_ConsumeAction();

#endif // IX_DEBUG_MENU_H
