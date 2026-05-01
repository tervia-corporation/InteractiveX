#ifndef IX_HUHLU_IX_MATERIAL_H
#define IX_HUHLU_IX_MATERIAL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IXMaterial
{
    char  name[64];
    float baseColor[4];
    float metallic;
    float roughness;
    float ao;
    float emissive[3];
    char  albedoTex[256];
    char  normalTex[256];
    char  mrTex[256];
    char  aoTex[256];
    char  emissiveTex[256];
} IXMaterial;

void IXMaterial_InitDefault(IXMaterial* m, const char* name);

#ifdef __cplusplus
}
#endif

#endif
