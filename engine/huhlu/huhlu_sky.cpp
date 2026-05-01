#include "huhlu_sky.h"
#if defined(_WIN32)
#include "../interface/style_iss.h"
#include "../renderer.h"
#include "../gui.h"
#endif

#if defined(_WIN32)
static int g_useImage = 0;
static char g_imagePath[512] = "";
static unsigned int g_top = 0xFF203050;
static unsigned int g_bottom = 0xFF5070A0;
#endif

void HuhluSky_ApplyFromISS(const char* selector)
{
#if !defined(_WIN32)
    (void)selector;
#else
    if (!selector) return;
    unsigned int c;
    if (ISS_GetColor(selector, "sky-top", &c)) g_top = c;
    if (ISS_GetColor(selector, "sky-bottom", &c)) g_bottom = c;
    if (ISS_GetBackgroundImage(selector, g_imagePath, (int)sizeof(g_imagePath)))
    {
        g_useImage = Renderer_LoadBackgroundImage(g_imagePath) ? 1 : 0;
    }
#endif
}

void HuhluSky_Render()
{
#if defined(_WIN32)
    if (g_useImage) return;
    int w,h; Renderer_GetClientSize(&w,&h);
    GUI_DrawGradientRect(0,0,w,h,g_top,g_bottom);
#endif
}
