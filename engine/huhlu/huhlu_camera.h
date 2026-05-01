#ifndef IX_HUHLU_CAMERA_H
#define IX_HUHLU_CAMERA_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum IXHuhluCameraMode { IX_CAM_ORBIT, IX_CAM_FPS, IX_CAM_FREE, IX_CAM_ORTHO, IX_CAM_PERSPECTIVE } IXHuhluCameraMode;

typedef struct IXHuhluCameraState
{
    IXHuhluCameraMode mode;
    float x,y,z;
    float yaw,pitch;
    float fov;
    float exposure;
    float gamma;
} IXHuhluCameraState;

void HuhluCamera_Init(IXHuhluCameraState* c, IXHuhluCameraMode mode);
void HuhluCamera_Update(IXHuhluCameraState* c, float moveX, float moveY, float moveZ, float lookX, float lookY);

#ifdef __cplusplus
}
#endif

#endif
