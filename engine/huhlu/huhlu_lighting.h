#ifndef IX_HUHLU_LIGHTING_H
#define IX_HUHLU_LIGHTING_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IXHuhluVec3 { float x,y,z; } IXHuhluVec3;

typedef struct IXHuhluDirectionalLight {
    IXHuhluVec3 direction;
    float intensity;
} IXHuhluDirectionalLight;

typedef struct IXHuhluPointLight {
    IXHuhluVec3 position;
    float range;
    float intensity;
} IXHuhluPointLight;

typedef struct IXHuhluAreaLight {
    IXHuhluVec3 position;
    IXHuhluVec3 normal;
    float radius;
    float intensity;
} IXHuhluAreaLight;

typedef struct IXHuhluSpotLight {
    IXHuhluVec3 position;
    IXHuhluVec3 direction;
    float range;
    float coneCos;
    float intensity;
} IXHuhluSpotLight;

float HuhluLight_Evaluate(IXHuhluVec3 p, IXHuhluVec3 n,
                          const IXHuhluDirectionalLight* dir,
                          const IXHuhluPointLight* point,
                          const IXHuhluSpotLight* spot,
                          const IXHuhluAreaLight* area,
                          float shadowFactor);

#ifdef __cplusplus
}
#endif

#endif
