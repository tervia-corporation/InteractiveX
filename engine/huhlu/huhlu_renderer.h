#ifndef IX_HUHLU_RENDERER_H
#define IX_HUHLU_RENDERER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IXHuhluCamera { float yaw, pitch, distance; } IXHuhluCamera;

int  HuhluRenderer_Initialize();
void HuhluRenderer_Shutdown();
void HuhluRenderer_DrawCube(const IXHuhluCamera* camera);
const char* HuhluRenderer_GetBackendName();

#ifdef __cplusplus
}
#endif

#endif
