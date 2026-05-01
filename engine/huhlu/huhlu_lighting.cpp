#include "huhlu_lighting.h"
#include <cmath>

static float dot(IXHuhluVec3 a, IXHuhluVec3 b){ return a.x*b.x + a.y*b.y + a.z*b.z; }
static IXHuhluVec3 sub(IXHuhluVec3 a, IXHuhluVec3 b){ IXHuhluVec3 r={a.x-b.x,a.y-b.y,a.z-b.z}; return r; }
static IXHuhluVec3 norm(IXHuhluVec3 v){ float l=std::sqrt(dot(v,v)); if(l<1e-5f){IXHuhluVec3 z={0,0,1};return z;} IXHuhluVec3 r={v.x/l,v.y/l,v.z/l}; return r; }

float HuhluLight_Evaluate(IXHuhluVec3 p, IXHuhluVec3 n,
                          const IXHuhluDirectionalLight* dir,
                          const IXHuhluPointLight* point,
                          const IXHuhluSpotLight* spot,
                          const IXHuhluAreaLight* area,
                          float shadowFactor)
{
    float i = 0.1f; // ambient
    IXHuhluVec3 nn = norm(n);
    if (dir)
    {
        IXHuhluVec3 ld = norm((IXHuhluVec3){-dir->direction.x,-dir->direction.y,-dir->direction.z});
        float ndl = dot(nn, ld); if (ndl > 0) i += ndl * dir->intensity;
    }
    if (point)
    {
        IXHuhluVec3 l = sub(point->position, p); float d2 = dot(l,l);
        float d = std::sqrt(d2);
        if (d < point->range && d > 1e-5f)
        {
            IXHuhluVec3 ln = {l.x/d,l.y/d,l.z/d};
            float att = 1.0f - (d / point->range);
            float ndl = dot(nn, ln); if (ndl > 0) i += ndl * att * point->intensity;
        }
    }
    if (spot)
    {
        IXHuhluVec3 l = sub(spot->position, p); float d2 = dot(l,l); float d = std::sqrt(d2);
        if (d < spot->range && d > 1e-5f)
        {
            IXHuhluVec3 ln = {l.x/d,l.y/d,l.z/d};
            IXHuhluVec3 sdir = norm((IXHuhluVec3){-spot->direction.x,-spot->direction.y,-spot->direction.z});
            float cone = dot(ln, sdir);
            if (cone >= spot->coneCos)
            {
                float att = 1.0f - (d / spot->range);
                float ndl = dot(nn, ln);
                if (ndl > 0) i += ndl * att * spot->intensity * cone;
            }
        }
    }
    if (area)
    {
        IXHuhluVec3 l = sub(area->position, p);
        float d = std::sqrt(dot(l,l));
        if (d < area->radius && d > 1e-5f)
        {
            IXHuhluVec3 ln = {l.x/d,l.y/d,l.z/d};
            float att = 1.0f - (d / area->radius);
            float ndl = dot(nn, ln);
            if (ndl > 0) i += ndl * att * area->intensity;
        }
    }
    if (shadowFactor < 0.0f) shadowFactor = 0.0f;
    if (shadowFactor > 1.0f) shadowFactor = 1.0f;
    i *= shadowFactor;
    if (i > 1.5f) i = 1.5f;
    return i;
}
