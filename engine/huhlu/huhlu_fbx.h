#ifndef IX_HUHLU_FBX_H
#define IX_HUHLU_FBX_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IXHuhluMesh
{
    float* vertices; // xyz xyz ...
    float* uvs;      // uv uv ... (aligned with triangulated indices)
    float* normals;  // xyz xyz ... (aligned with triangulated indices)
    int vertexCount;
    int* indices;    // triangle indices
    int indexCount;
    char texturePath[512];
} IXHuhluMesh;

int  HuhluFBX_LoadBasicAscii(const char* path, IXHuhluMesh* outMesh);
void HuhluFBX_FreeMesh(IXHuhluMesh* mesh);

#ifdef __cplusplus
}
#endif

#endif
