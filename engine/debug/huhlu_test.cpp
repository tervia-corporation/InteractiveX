#include "huhlu_test.h"
#include "../huhlu/huhlu_renderer.h"
#include "../huhlu/huhlu_fire3d.h"
#include "../huhlu/huhlu_sky.h"
#include "../huhlu/huhlu_postfx.h"
#include "../huhlu/huhlu_particles.h"
#include "../huhlu/huhlu_physics.h"
#include "../renderer.h"
#include <cstdio>

static IXHuhluCamera g_cam = {0.4f, 0.3f, 4.0f};
static IXHuhluPostFX g_fx;
static int g_back = 0;
static int g_autoOrbit = 1;
static IXRigidBody3D g_rb = {0,1.2f,0, 0.8f,0,0, 1.0f,1};
static float g_lastRayT = -1.0f;

void DebugHuhlu_Initialize(){ g_back=0; HuhluRenderer_Initialize(); HuhluFire3D_Initialize(24,24,16); HuhluSky_ApplyFromISS("#huhlu_sky"); HuhluPostFX_Init(&g_fx); HuhluParticles_ResetExplosion(0,0,0); }
void DebugHuhlu_Update(int,int,int,int){ if(g_autoOrbit) g_cam.yaw += 0.01f; HuhluPhysics_ApplyGravity(&g_rb,0.016f,9.8f); HuhluPhysics_Integrate(&g_rb,0.016f); if(g_rb.y < -1.0f){ g_rb.y=-1.0f; g_rb.vy *= -0.55f; } IXRay3D r={0,0,-5,0,0,1}; IXAABB3D b={g_rb.x-0.25f,g_rb.y-0.25f,g_rb.z-0.25f,0.5f,0.5f,0.5f}; HuhluPhysics_RaycastAABB3D(r,b,&g_lastRayT); HuhluFire3D_Update(0.016f); HuhluParticles_Update(0.016f); }
void DebugHuhlu_HandleKeyDown(unsigned int k){ if(k==0x1B) g_back=1; if(k=='A') g_cam.yaw-=0.08f; if(k=='D') g_cam.yaw+=0.08f; if(k=='W') g_cam.pitch+=0.08f; if(k=='S') g_cam.pitch-=0.08f; if(k=='F') HuhluFire3D_SetEnabled(!HuhluFire3D_IsEnabled()); if(k=='E') HuhluParticles_ResetExplosion(g_rb.x,g_rb.y,g_rb.z); if(k=='R'){ g_rb.x=0;g_rb.y=1.2f;g_rb.z=0;g_rb.vx=0.8f;g_rb.vy=0;g_rb.vz=0; } if(k=='O') g_autoOrbit=!g_autoOrbit; if(k=='P') HuhluPostFX_ToggleAdvanced(&g_fx); }
int DebugHuhlu_ShouldReturnToMenu(){ return g_back; }
void DebugHuhlu_ClearReturnToMenu(){ g_back=0; }

void DebugHuhlu_Render(){ int w,h; Renderer_GetClientSize(&w,&h); HuhluSky_Render(); HuhluRenderer_DrawCube(&g_cam); HuhluFire3D_Render(&g_cam); HuhluParticles_Render(g_cam.yaw,g_cam.pitch,g_cam.distance); HuhluPostFX_ApplyOverlay(&g_fx,w,h); }

void DebugHuhlu_ApplyRendererState(float fps,float frameMs){
    char st[256]; std::snprintf(st,sizeof(st),"Huhlu+ | %s | FPS %.1f | %.2fms | RB(%.2f,%.2f,%.2f) RayT=%.2f | Bloom:%d SSAO:%d",HuhluRenderer_GetBackendName(),fps,frameMs,g_rb.x,g_rb.y,g_rb.z,g_lastRayT,g_fx.bloom,g_fx.ssao);
    Renderer_SetOverlayText("InteractiveX", "Huhlu Native Backend", st);
    Renderer_SetHelpText("WASD rotate | O auto-orbit | R reset | F fire3D | E explosion | P postfx | ESC");
    Renderer_SetDisplayOptions(1,1,1);
    Renderer_SetControlButtons(NULL,0); Renderer_SetControlSliders(NULL,0);
}
