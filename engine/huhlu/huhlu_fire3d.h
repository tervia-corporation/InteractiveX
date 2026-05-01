#ifndef IX_HUHLU_FIRE3D_H
#define IX_HUHLU_FIRE3D_H

#include "huhlu_renderer.h"

#ifdef __cplusplus
extern "C" {
#endif

void HuhluFire3D_Initialize(int width, int height, int depth);
void HuhluFire3D_Update(float dt);
void HuhluFire3D_Render(const IXHuhluCamera* cam);
void HuhluFire3D_SetEnabled(int enabled);
int  HuhluFire3D_IsEnabled();

#ifdef __cplusplus
}
#endif

#endif
