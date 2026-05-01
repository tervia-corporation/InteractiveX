#include "huhlu_postfx.h"
#if defined(_WIN32)
#include "../gui.h"
#endif

void HuhluPostFX_Init(IXHuhluPostFX* fx)
{
    if(!fx) return;
    fx->exposure=1.0f; fx->gamma=2.2f; fx->vignette=1; fx->filmGrain=0;
    fx->bloom=1; fx->tonemap=1; fx->ssao=1; fx->colorGrading=1;
    fx->bloomStrength=0.35f; fx->ssaoStrength=0.25f;
}

void HuhluPostFX_ToggleAdvanced(IXHuhluPostFX* fx)
{
    if(!fx) return;
    fx->bloom = !fx->bloom;
    fx->ssao = !fx->ssao;
    fx->colorGrading = !fx->colorGrading;
}

void HuhluPostFX_ApplyOverlay(const IXHuhluPostFX* fx, int w, int h)
{
#if defined(_WIN32)
    if (!fx) return;
    if (fx->vignette){ GUI_DrawFilledRect(0,0,w,12,0x22000000); GUI_DrawFilledRect(0,h-12,w,12,0x22000000); }
    if (fx->bloom){ GUI_DrawFilledRect(0,0,w,2,0x11FFD080); GUI_DrawFilledRect(0,h-2,w,2,0x11FFD080); }
    if (fx->ssao){ GUI_DrawFilledRect(0,0,2,h,0x11000000); GUI_DrawFilledRect(w-2,0,2,h,0x11000000); }
#else
    (void)fx; (void)w; (void)h;
#endif
}
