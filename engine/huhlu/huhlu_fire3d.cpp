#include "huhlu_fire3d.h"
#include <cstdlib>
#include <cstring>
#include <cmath>
#if defined(_WIN32)
#include "../gui.h"
#include "../renderer.h"
#endif

static int g_w=24,g_h=24,g_d=16,g_enabled=0;
static unsigned char* g_heat = 0;
static int idx3(int x,int y,int z){ return z*g_w*g_h + y*g_w + x; }

void HuhluFire3D_Initialize(int w, int h, int d){ g_w=w>4?w:24; g_h=h>4?h:24; g_d=d>2?d:16; if(g_heat) std::free(g_heat); g_heat=(unsigned char*)std::malloc((size_t)g_w*g_h*g_d); if(g_heat) std::memset(g_heat,0,(size_t)g_w*g_h*g_d);} 
void HuhluFire3D_SetEnabled(int e){ g_enabled=e?1:0; }
int HuhluFire3D_IsEnabled(){ return g_enabled; }

void HuhluFire3D_Update(float dt){ (void)dt; if(!g_enabled||!g_heat) return; for(int z=0;z<g_d;++z) for(int x=0;x<g_w;++x) g_heat[idx3(x,g_h-1,z)] = (unsigned char)(180+(std::rand()%75)); for(int y=g_h-2;y>=0;--y) for(int z=0;z<g_d;++z) for(int x=0;x<g_w;++x){ int s=g_heat[idx3(x,y+1,z)]; if(x>0)s+=g_heat[idx3(x-1,y+1,z)]; if(x<g_w-1)s+=g_heat[idx3(x+1,y+1,z)]; if(z>0)s+=g_heat[idx3(x,y+1,z-1)]; if(z<g_d-1)s+=g_heat[idx3(x,y+1,z+1)]; s/=5; if(s>3)s-=(std::rand()%4); g_heat[idx3(x,y,z)]=(unsigned char)s; } }

void HuhluFire3D_Render(const IXHuhluCamera* cam)
{
#if !defined(_WIN32)
    (void)cam;
#else
    if(!g_enabled||!g_heat||!cam) return; int sw,sh; Renderer_GetClientSize(&sw,&sh);
    for(int y=0;y<g_h;y++) for(int z=0;z<g_d;z++) for(int x=0;x<g_w;x++){
        unsigned char h=g_heat[idx3(x,y,z)]; if(h<40) continue;
        float wx=(x-g_w*0.5f)*0.08f, wy=(g_h-y)*0.06f-0.8f, wz=(z-g_d*0.5f)*0.08f;
        float cy=cosf(cam->yaw), sy=sinf(cam->yaw), cx=cosf(cam->pitch), sx=sinf(cam->pitch);
        float rx=wx*cy-wz*sy, rz=wx*sy+wz*cy, ry=wy*cx-rz*sx; rz=wy*sx+rz*cx; rz+=cam->distance; if(rz<=0.1f) continue;
        float f=260.0f/rz; int sxp=(int)(sw*0.65f+rx*f), syp=(int)(sh*0.7f-ry*f); unsigned int col=0xFF000000u|((unsigned int)h<<16)|((unsigned int)(h>180?180:h)<<8); int szp=(int)(1+f*0.02f); if(szp<1)szp=1; if(szp>4)szp=4; GUI_DrawFilledRect(sxp,syp,szp,szp,col);
    }
#endif
}
