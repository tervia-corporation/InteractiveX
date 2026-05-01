#ifndef IX_HUHLU_SCENE_IXS_H
#define IX_HUHLU_SCENE_IXS_H

#include "ix_material.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IXHuhluSceneObject
{
    char meshPath[256];
    IXMaterial material;
    float tx,ty,tz;
} IXHuhluSceneObject;

int HuhluSceneIXS_Load(const char* path, IXHuhluSceneObject* objects, int maxObjects);

#ifdef __cplusplus
}
#endif

#endif
