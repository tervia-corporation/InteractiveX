#ifndef IX_HUHLU_POSTFX_H
#define IX_HUHLU_POSTFX_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct IXHuhluPostFX {
    float exposure;
    float gamma;
    int vignette;
    int filmGrain;
    int bloom;
    int tonemap;
    int ssao;
    int colorGrading;
    float bloomStrength;
    float ssaoStrength;
} IXHuhluPostFX;

void HuhluPostFX_Init(IXHuhluPostFX* fx);
void HuhluPostFX_ApplyOverlay(const IXHuhluPostFX* fx, int w, int h);
void HuhluPostFX_ToggleAdvanced(IXHuhluPostFX* fx);

#ifdef __cplusplus
}
#endif
#endif
