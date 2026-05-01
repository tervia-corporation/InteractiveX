#include "huhlu_camera.h"
#include <cstring>

void HuhluCamera_Init(IXHuhluCameraState* c, IXHuhluCameraMode mode)
{
    if(!c) return; std::memset(c,0,sizeof(*c)); c->mode=mode; c->z=-4.0f; c->fov=70.0f; c->exposure=1.0f; c->gamma=2.2f;
}

void HuhluCamera_Update(IXHuhluCameraState* c, float moveX, float moveY, float moveZ, float lookX, float lookY)
{
    if(!c) return; c->yaw += lookX; c->pitch += lookY;
    if(c->mode==IX_CAM_ORTHO){ c->x += moveX*0.02f; c->y += moveY*0.02f; }
    else { c->x += moveX*0.05f; c->y += moveY*0.05f; c->z += moveZ*0.05f; }
}
