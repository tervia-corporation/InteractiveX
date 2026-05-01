#include "huhlu_particles.h"
#if defined(_WIN32)
#include "../gui.h"
#include "../renderer.h"
#endif
#include <cstdlib>
#include <cmath>

struct P { float x,y,z,vx,vy,vz,life; };
static P g_p[128];

void HuhluParticles_ResetExplosion(float x,float y,float z){ for(int i=0;i<128;i++){ g_p[i].x=x;g_p[i].y=y;g_p[i].z=z; g_p[i].vx=((std::rand()%200)-100)/120.0f; g_p[i].vy=((std::rand()%200)-40)/120.0f; g_p[i].vz=((std::rand()%200)-100)/120.0f; g_p[i].life=1.0f+(std::rand()%100)/100.0f; } }
void HuhluParticles_Update(float dt){ for(int i=0;i<128;i++){ if(g_p[i].life<=0) continue; g_p[i].life-=dt; g_p[i].vy-=2.8f*dt; g_p[i].x+=g_p[i].vx*dt; g_p[i].y+=g_p[i].vy*dt; g_p[i].z+=g_p[i].vz*dt; } }
void HuhluParticles_Render(float yaw,float pitch,float dist){
#if defined(_WIN32)
 int sw,sh; Renderer_GetClientSize(&sw,&sh); float cy=cosf(yaw), sy=sinf(yaw), cx=cosf(pitch), sx=sinf(pitch);
 for(int i=0;i<128;i++){ if(g_p[i].life<=0) continue; float rx=g_p[i].x*cy-g_p[i].z*sy, rz=g_p[i].x*sy+g_p[i].z*cy, ry=g_p[i].y*cx-rz*sx; rz=g_p[i].y*sx+rz*cx; rz+=dist; if(rz<=0.1f) continue; float f=280.0f/rz; int x=(int)(sw*0.65f+rx*f), y=(int)(sh*0.7f-ry*f); unsigned int a=(unsigned int)(g_p[i].life*255); if(a>255)a=255; GUI_DrawFilledRect(x,y,2,2,(a<<24)|0x00FFA040u); }
#else
 (void)yaw;(void)pitch;(void)dist;
#endif
}
