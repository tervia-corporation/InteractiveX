#ifndef IX_HUHLU_GLTF_H
#define IX_HUHLU_GLTF_H

#include "huhlu_fbx.h"

#ifdef __cplusplus
extern "C" {
#endif

int HuhluGLTF_LoadSimple(const char* path, IXHuhluMesh* outMesh); // gltf json
int HuhluGLB_LoadSimple(const char* path, IXHuhluMesh* outMesh);  // glb wrapper (stub)

#ifdef __cplusplus
}
#endif

#endif
