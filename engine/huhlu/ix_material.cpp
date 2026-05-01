#include "ix_material.h"
#include <cstring>
#include <cstdio>

void IXMaterial_InitDefault(IXMaterial* m, const char* name)
{
    if (!m) return;
    std::memset(m, 0, sizeof(*m));
    std::snprintf(m->name, sizeof(m->name), "%s", name ? name : "IXMaterial");
    m->baseColor[0]=1.0f; m->baseColor[1]=1.0f; m->baseColor[2]=1.0f; m->baseColor[3]=1.0f;
    m->metallic=0.0f; m->roughness=0.7f; m->ao=1.0f;
}
