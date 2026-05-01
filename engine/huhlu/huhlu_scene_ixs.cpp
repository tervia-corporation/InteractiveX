#include "huhlu_scene_ixs.h"
#include <cstdio>
#include <cstring>

int HuhluSceneIXS_Load(const char* path, IXHuhluSceneObject* objects, int maxObjects)
{
    if(!path||!objects||maxObjects<=0) return 0;
    FILE* f=std::fopen(path,"rb"); if(!f) return 0;
    int n=0; char line[512];
    while(std::fgets(line,sizeof(line),f) && n<maxObjects)
    {
        if(line[0]=='#' || line[0]=='\n') continue;
        IXHuhluSceneObject* o=&objects[n]; std::memset(o,0,sizeof(*o)); IXMaterial_InitDefault(&o->material,"default");
        // formato simples: mesh=...; material=...; pos=x,y,z
        std::sscanf(line,"mesh=%255[^;]; material=%63[^;]; pos=%f,%f,%f",o->meshPath,o->material.name,&o->tx,&o->ty,&o->tz);
        if(o->meshPath[0]) n++;
    }
    std::fclose(f); return n;
}
