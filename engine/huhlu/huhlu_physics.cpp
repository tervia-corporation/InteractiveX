#include "huhlu_physics.h"
int HuhluPhysics_OverlapAABB2D(IXAABB2D a, IXAABB2D b){ return (a.x < b.x+b.w && a.x+a.w > b.x && a.y < b.y+b.h && a.y+a.h > b.y) ? 1 : 0; }
int HuhluPhysics_OverlapAABB3D(IXAABB3D a, IXAABB3D b){ return (a.x < b.x+b.w && a.x+a.w > b.x && a.y < b.y+b.h && a.y+a.h > b.y && a.z < b.z+b.d && a.z+a.d > b.z) ? 1 : 0; }

#include <cmath>
int HuhluPhysics_OverlapSphere(IXSphere a, IXSphere b){ float dx=a.x-b.x,dy=a.y-b.y,dz=a.z-b.z; float rr=a.r+b.r; return (dx*dx+dy*dy+dz*dz)<=rr*rr?1:0; }
int HuhluPhysics_RaycastAABB3D(IXRay3D r, IXAABB3D b, float* outT){ float tmin=0.0f,tmax=1e9f; float minv[3]={b.x,b.y,b.z}, maxv[3]={b.x+b.w,b.y+b.h,b.z+b.d}; float o[3]={r.ox,r.oy,r.oz}, d[3]={r.dx,r.dy,r.dz}; for(int i=0;i<3;i++){ if (std::fabs(d[i])<1e-6f){ if(o[i]<minv[i]||o[i]>maxv[i]) return 0; } else { float inv=1.0f/d[i]; float t1=(minv[i]-o[i])*inv,t2=(maxv[i]-o[i])*inv; if(t1>t2){float t=t1;t1=t2;t2=t;} if(t1>tmin)tmin=t1; if(t2<tmax)tmax=t2; if(tmin>tmax) return 0; } } if(outT)*outT=tmin; return 1; }
void HuhluPhysics_ApplyGravity(IXRigidBody3D* rb, float dt, float gravity){ if(!rb||!rb->useGravity) return; rb->vy -= gravity*dt; }
void HuhluPhysics_Integrate(IXRigidBody3D* rb, float dt){ if(!rb) return; rb->x += rb->vx*dt; rb->y += rb->vy*dt; rb->z += rb->vz*dt; }
