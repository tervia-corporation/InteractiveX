#ifndef IX_HUHLU_BACKEND_COMMON_H
#define IX_HUHLU_BACKEND_COMMON_H

#include "huhlu_renderer.h"
#include "huhlu_lighting.h"
#if defined(_WIN32)
#include "../gui.h"
#include "../renderer.h"
#include <cmath>
#include <cstdlib>

static inline void Huhlu_Line(int x1,int y1,int x2,int y2,unsigned int c){int dx=x2-x1,dy=y2-y1,s=abs(dx)>abs(dy)?abs(dx):abs(dy);if(s<1)s=1;for(int i=0;i<=s;i++)GUI_DrawFilledRect(x1+dx*i/s,y1+dy*i/s,2,2,c);} 
static inline void Huhlu_DrawSoftwareCube(const IXHuhluCamera* c){ if(!c) return; int sw,sh; Renderer_GetClientSize(&sw,&sh); float p[8][3]={{-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},{-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1}}; int e[12][2]={{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}}; int ok[8],sx[8],sy[8]; IXHuhluVec3 wp[8]; float cy=cosf(c->yaw),syaw=sinf(c->yaw),cx=cosf(c->pitch),sxp=sinf(c->pitch); for(int i=0;i<8;i++){float x=p[i][0]*cy-p[i][2]*syaw;float z=p[i][0]*syaw+p[i][2]*cy;float y=p[i][1]*cx-z*sxp;z=p[i][1]*sxp+z*cx; wp[i].x=x; wp[i].y=y; wp[i].z=z; z+=c->distance; if(z<=0.1f){ok[i]=0;continue;} float f=300.0f/z; sx[i]=(int)(sw*0.65f+x*f); sy[i]=(int)(sh*0.5f-y*f); ok[i]=1;} IXHuhluDirectionalLight dl={{-0.5f,-1.0f,-0.3f},0.6f}; IXHuhluPointLight pl={{1.5f,1.5f,2.0f},6.0f,0.8f}; IXHuhluSpotLight sl={{-2.0f,2.0f,1.0f},{1.0f,-1.0f,0.0f},8.0f,0.55f,0.9f}; IXHuhluAreaLight al={{0.0f,2.5f,0.0f},{0,-1,0},5.0f,0.5f}; for(int i=0;i<12;i++){int a=e[i][0],b=e[i][1]; if(ok[a]&&ok[b]){ IXHuhluVec3 m={(wp[a].x+wp[b].x)*0.5f,(wp[a].y+wp[b].y)*0.5f,(wp[a].z+wp[b].z)*0.5f}; IXHuhluVec3 n=m; float simpleShadow = (m.y < -0.7f) ? 0.7f : 1.0f; float li=HuhluLight_Evaluate(m,n,&dl,&pl,&sl,&al,simpleShadow); unsigned int c0=(unsigned int)(127*li); if(c0>255)c0=255; unsigned int col=0xFF000000u | (c0<<16) | ((unsigned int)(215*li>255?255:215*li)<<8) | 255u; Huhlu_Line(sx[a],sy[a],sx[b],sy[b],col);} } }
#else
static inline void Huhlu_DrawSoftwareCube(const IXHuhluCamera* c){ (void)c; }
#endif

#endif
