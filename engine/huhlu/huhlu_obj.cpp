#include "huhlu_obj.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

int HuhluOBJ_LoadSimple(const char* path, IXHuhluMesh* out)
{
    if(!path||!out) return 0; HuhluFBX_FreeMesh(out);
    FILE* f=std::fopen(path,"rb"); if(!f) return 0;
    out->vertices=(float*)std::malloc(sizeof(float)*3*65536);
    out->indices=(int*)std::malloc(sizeof(int)*3*65536);
    out->uvs=(float*)std::calloc((size_t)2*65536,sizeof(float));
    out->normals=(float*)std::calloc((size_t)3*65536,sizeof(float));
    if(!out->vertices||!out->indices||!out->uvs||!out->normals){ std::fclose(f); HuhluFBX_FreeMesh(out); return 0; }
    int vc=0,ic=0; char line[256];
    while(std::fgets(line,sizeof(line),f)){
        if(line[0]=='v'&&line[1]==' '){ float x,y,z; if(std::sscanf(line,"v %f %f %f",&x,&y,&z)==3){ out->vertices[vc*3]=x; out->vertices[vc*3+1]=y; out->vertices[vc*3+2]=z; vc++; }}
        else if(line[0]=='f'&&line[1]==' '){ int a,b,c; if(std::sscanf(line,"f %d %d %d",&a,&b,&c)==3){ out->indices[ic++]=a-1; out->indices[ic++]=b-1; out->indices[ic++]=c-1; }}
    }
    std::fclose(f); out->vertexCount=vc; out->indexCount=ic; return vc>0&&ic>0;
}
