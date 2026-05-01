#ifndef IX_HUHLU_PHYSICS_H
#define IX_HUHLU_PHYSICS_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct IXAABB2D { float x,y,w,h; } IXAABB2D;
typedef struct IXAABB3D { float x,y,z,w,h,d; } IXAABB3D;
int HuhluPhysics_OverlapAABB2D(IXAABB2D a, IXAABB2D b);
int HuhluPhysics_OverlapAABB3D(IXAABB3D a, IXAABB3D b);

typedef struct IXSphere { float x,y,z,r; } IXSphere;
typedef struct IXRay3D { float ox,oy,oz, dx,dy,dz; } IXRay3D;
typedef struct IXRigidBody3D { float x,y,z, vx,vy,vz, mass; int useGravity; } IXRigidBody3D;

int HuhluPhysics_OverlapSphere(IXSphere a, IXSphere b);
int HuhluPhysics_RaycastAABB3D(IXRay3D r, IXAABB3D b, float* outT);
void HuhluPhysics_ApplyGravity(IXRigidBody3D* rb, float dt, float gravity);
void HuhluPhysics_Integrate(IXRigidBody3D* rb, float dt);

#ifdef __cplusplus
}
#endif
#endif
