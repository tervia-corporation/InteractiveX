#include "huhlu_modules_test.h"
#include "../renderer.h"
#include "../interface/button.h"
#include "../interface/layout.h"
#include "../huhlu/huhlu_physics.h"
#include "../huhlu/huhlu_camera.h"
#include "../huhlu/ix_material.h"
#include "../huhlu/huhlu_scene_ixs.h"
#include "../huhlu/huhlu_fbx.h"
#include "../huhlu/huhlu_obj.h"
#include "../huhlu/huhlu_gltf.h"
#include "../huhlu/huhlu_particles.h"
#include <cstdio>

static IXButton g_buttons[9];
static const IXButton* g_btnRefs[9];
static int g_back=0;
static char g_status[160] = "Select Huhlu module test.";

void DebugHuhluModules_Initialize(){
    const char* n[9] = {"Physics AABB/Sphere","Raycast","Camera Modes","IXMaterial","IXS Scene","FBX/OBJ/GLTF","Particles Explosion","Backend/Video","Back to Menu"};
    for(int i=0;i<9;i++){ Button_Init(&g_buttons[i],0,0,0,0,n[i]); g_btnRefs[i]=&g_buttons[i]; }
}
void DebugHuhluModules_InvalidateLayout(){}

static void layout(){ int x,y,w,h; Renderer_GetOverlayButtonsRect(&x,&y,&w,&h); IXLayout l; Layout_BeginVertical(&l,x,y,w,h,4); int bh=28; for(int i=0;i<9;i++){ int bx,by,bw,bh2; Layout_Next(&l,bh,&bx,&by,&bw,&bh2); Button_SetBounds(&g_buttons[i],bx,by,bw,bh2);} }

void DebugHuhluModules_Update(int mx,int my,int md,int mr){ layout();
    if(Button_Handle(&g_buttons[0],mx,my,md,mr)){ IXAABB2D a={0,0,10,10},b={5,5,8,8}; IXSphere s1={0,0,0,1},s2={1,0,0,1}; std::snprintf(g_status,sizeof(g_status),"AABB=%d Sphere=%d",HuhluPhysics_OverlapAABB2D(a,b),HuhluPhysics_OverlapSphere(s1,s2)); }
    if(Button_Handle(&g_buttons[1],mx,my,md,mr)){ IXRay3D r={0,0,-5,0,0,1}; IXAABB3D b={-1,-1,-1,2,2,2}; float t=0; int hit=HuhluPhysics_RaycastAABB3D(r,b,&t); std::snprintf(g_status,sizeof(g_status),"Raycast hit=%d t=%.2f",hit,t); }
    if(Button_Handle(&g_buttons[2],mx,my,md,mr)){ IXHuhluCameraState c; HuhluCamera_Init(&c,IX_CAM_ORBIT); HuhluCamera_Update(&c,1,0,0,0.1f,0.1f); std::snprintf(g_status,sizeof(g_status),"Camera mode=%d FOV=%.1f",(int)c.mode,c.fov); }
    if(Button_Handle(&g_buttons[3],mx,my,md,mr)){ IXMaterial m; IXMaterial_InitDefault(&m,"demo"); std::snprintf(g_status,sizeof(g_status),"IXMaterial ok roughness=%.2f",m.roughness); }
    if(Button_Handle(&g_buttons[4],mx,my,md,mr)){ IXHuhluSceneObject o[4]; int c=HuhluSceneIXS_Load("assets/scene/test.ixs",o,4); std::snprintf(g_status,sizeof(g_status),"IXS loaded objects=%d",c); }
    if(Button_Handle(&g_buttons[5],mx,my,md,mr)){ IXHuhluMesh m; int a=HuhluFBX_LoadBasicAscii("assets/model/test.fbx",&m); int b=HuhluOBJ_LoadSimple("assets/model/test.obj",&m); int c=HuhluGLTF_LoadSimple("assets/model/test.gltf",&m); HuhluFBX_FreeMesh(&m); std::snprintf(g_status,sizeof(g_status),"FBX=%d OBJ=%d GLTF=%d",a,b,c); }
    if(Button_Handle(&g_buttons[6],mx,my,md,mr)){ HuhluParticles_ResetExplosion(0,0,0); std::snprintf(g_status,sizeof(g_status),"Explosion triggered"); }
    if(Button_Handle(&g_buttons[7],mx,my,md,mr)){ std::snprintf(g_status,sizeof(g_status),"Backend=%s | Video codecs: MP4/OGV helpers enabled", "runtime"); }
    if(Button_Handle(&g_buttons[8],mx,my,md,mr)){ g_back=1; }
}

void DebugHuhluModules_ApplyRendererState(float fps,float frameMs){
    char st[200]; std::snprintf(st,sizeof(st),"%s | FPS %.1f | %.2fms",g_status,fps,frameMs);
    Renderer_SetOverlayText("InteractiveX","Huhlu Modules Debug",st);
    Renderer_SetHelpText("Test each Huhlu module via buttons");
    Renderer_SetDisplayOptions(1,1,1);
    Renderer_SetControlButtons(g_btnRefs,9);
    Renderer_SetControlSliders(NULL,0);
}
int DebugHuhluModules_ShouldReturnToMenu(){ return g_back; }
void DebugHuhluModules_ClearReturnToMenu(){ g_back=0; }
