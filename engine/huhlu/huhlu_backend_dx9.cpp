#include "huhlu_renderer.h"
#include "../gui.h"
#include "../renderer.h"
#include <cmath>

#if IX_ENABLE_DX9
static void line(int x1,int y1,int x2,int y2,unsigned int c){int dx=x2-x1,dy=y2-y1,s=abs(dx)>abs(dy)?abs(dx):abs(dy);if(s<1)s=1;for(int i=0;i<=s;i++)GUI_DrawFilledRect(x1+dx*i/s,y1+dy*i/s,2,2,c);} 
static void rot(float yaw,float pitch,float x,float y,float z,float* ox,float* oy,float* oz){float cy=cosf(yaw),sy=sinf(yaw),cx=cosf(pitch),sx=sinf(pitch);float rx=x*cy-z*sy,rz=x*sy+z*cy;*oy=y*cx-rz*sx;*oz=y*sx+rz*cx;*ox=rx;}
extern "C" int HuhluRenderer_Initialize(){return 1;} extern "C" void HuhluRenderer_Shutdown(){}
extern "C" const char* HuhluRenderer_GetBackendName(){return "DX9";}
extern "C" void HuhluRenderer_DrawCube(const IXHuhluCamera* c){if(!c)return;int sw,sh;Renderer_GetClientSize(&sw,&sh);float p[8][3]={{-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},{-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1}};int e[12][2]={{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};int ok[8],sx[8],sy[8];for(int i=0;i<8;i++){float x,y,z;rot(c->yaw,c->pitch,p[i][0],p[i][1],p[i][2],&x,&y,&z);z+=c->distance;if(z<=0.1f){ok[i]=0;continue;}float f=300.0f/z;sx[i]=(int)(sw*0.65f+x*f);sy[i]=(int)(sh*0.5f-y*f);ok[i]=1;}for(int i=0;i<12;i++){int a=e[i][0],b=e[i][1];if(ok[a]&&ok[b])line(sx[a],sy[a],sx[b],sy[b],0xFF7FD7FF);} }
#endif
