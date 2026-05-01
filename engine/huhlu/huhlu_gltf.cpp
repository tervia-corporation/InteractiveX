#include "huhlu_gltf.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

int HuhluGLTF_LoadSimple(const char* path, IXHuhluMesh* out)
{
    if(!path||!out) return 0;
    // loader simplificado: aceita arquivo txt com linhas "v x y z" e "f a b c"
    FILE* f=std::fopen(path,"rb"); if(!f) return 0;
    HuhluFBX_FreeMesh(out);
    out->vertices=(float*)std::malloc(sizeof(float)*3*65536);
    out->indices=(int*)std::malloc(sizeof(int)*3*65536);
    out->uvs=(float*)std::malloc(sizeof(float)*2*65536);
    out->normals=(float*)std::malloc(sizeof(float)*3*65536);
    if(!out->vertices||!out->indices||!out->uvs||!out->normals){ std::fclose(f); HuhluFBX_FreeMesh(out); return 0; }
    int vc=0,ic=0; char line[256];
    while(std::fgets(line,sizeof(line),f)){
        if(line[0]=='v'&&line[1]==' '){ float x,y,z; if(std::sscanf(line,"v %f %f %f",&x,&y,&z)==3){ out->vertices[vc*3+0]=x; out->vertices[vc*3+1]=y; out->vertices[vc*3+2]=z; vc++; }}
        else if(line[0]=='f'&&line[1]==' '){ int a,b,c; if(std::sscanf(line,"f %d %d %d",&a,&b,&c)==3){ out->indices[ic++]=a; out->indices[ic++]=b; out->indices[ic++]=c; }}
    }
    std::fclose(f); out->vertexCount=vc; out->indexCount=ic; return vc>0&&ic>0;
}

int HuhluGLB_LoadSimple(const char* path, IXHuhluMesh* outMesh)
{
    // wrapper minimo: para GLB real ainda pendente
    return HuhluGLTF_LoadSimple(path, outMesh);
}
