#include "huhlu_fbx.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

static char* find_after(char* haystack, const char* key)
{
    char* p = std::strstr(haystack, key);
    if (!p) return 0;
    p = std::strchr(p, 'a');
    if (!p) return 0;
    p = std::strchr(p, ':');
    return p ? p + 1 : 0;
}

void HuhluFBX_FreeMesh(IXHuhluMesh* m)
{
    if (!m) return;
    if (m->vertices) std::free(m->vertices);
    if (m->uvs) std::free(m->uvs);
    if (m->normals) std::free(m->normals);
    if (m->indices) std::free(m->indices);
    m->vertices = 0; m->uvs = 0; m->normals = 0; m->indices = 0; m->vertexCount = 0; m->indexCount = 0; m->texturePath[0]=0;
}

int HuhluFBX_LoadBasicAscii(const char* path, IXHuhluMesh* out)
{
    if (!path || !out) return 0;
    HuhluFBX_FreeMesh(out);
    FILE* f = std::fopen(path, "rb");
    if (!f) return 0;

    char* data = 0; long sz=0;
    std::fseek(f,0,SEEK_END); sz=std::ftell(f); std::fseek(f,0,SEEK_SET);
    data = (char*)std::malloc((size_t)sz + 1); if(!data){std::fclose(f);return 0;}
    std::fread(data,1,(size_t)sz,f); data[sz]=0; std::fclose(f);

    char* vSec = std::strstr(data, "Vertices:");
    char* pSec = std::strstr(data, "PolygonVertexIndex:");
    if (!vSec || !pSec){ std::free(data); return 0; }

    int vcount = std::atoi(std::strchr(vSec, '*')+1) / 3;
    int polyCount = std::atoi(std::strchr(pSec, '*')+1);
    if (vcount <= 0 || polyCount <= 0){ std::free(data); return 0; }

    out->vertices = (float*)std::malloc((size_t)vcount * 3 * sizeof(float));
    out->indices  = (int*)std::malloc((size_t)polyCount * 3 * sizeof(int));
    out->uvs      = (float*)std::malloc((size_t)polyCount * 3 * 2 * sizeof(float));
    out->normals  = (float*)std::malloc((size_t)polyCount * 3 * 3 * sizeof(float));
    if (!out->vertices || !out->indices || !out->uvs || !out->normals){ HuhluFBX_FreeMesh(out); std::free(data); return 0; }

    char* va = find_after(vSec, "a:"); if(!va){ HuhluFBX_FreeMesh(out); std::free(data); return 0; }
    for (int i=0;i<vcount*3;i++) out->vertices[i] = (float)std::strtod(va, &va);

    // Optional normal data
    float* nrmData = 0; int nrmDataCount = 0;
    int* nrmIndexData = 0; int nrmIndexCount = 0;
    char* nSec = std::strstr(data, "Normals:");
    char* nISec = std::strstr(data, "NormalsIndex:");
    if (nSec)
    {
        nrmDataCount = std::atoi(std::strchr(nSec, '*')+1);
        if (nrmDataCount > 2)
        {
            nrmData = (float*)std::malloc((size_t)nrmDataCount * sizeof(float));
            if (nISec) { nrmIndexCount = std::atoi(std::strchr(nISec, '*')+1); if (nrmIndexCount > 0) nrmIndexData = (int*)std::malloc((size_t)nrmIndexCount * sizeof(int)); }
            if (nrmData)
            {
                char* na = find_after(nSec, "a:");
                for (int i=0;i<nrmDataCount;i++) nrmData[i] = (float)std::strtod(na, &na);
                if (nrmIndexData && nISec) { char* ni = find_after(nISec, "a:"); for (int i=0;i<nrmIndexCount;i++) nrmIndexData[i]=(int)std::strtol(ni,&ni,10); }
            }
        }
    }

    // Optional UV data
    float* uvData = 0; int uvDataCount = 0;
    int* uvIndexData = 0; int uvIndexCount = 0;
    char* uvSec = std::strstr(data, "UV:");
    char* uvISec = std::strstr(data, "UVIndex:");
    if (uvSec && uvISec)
    {
        uvDataCount = std::atoi(std::strchr(uvSec, '*')+1);
        uvIndexCount = std::atoi(std::strchr(uvISec, '*')+1);
        if (uvDataCount > 1 && uvIndexCount > 0)
        {
            uvData = (float*)std::malloc((size_t)uvDataCount * sizeof(float));
            uvIndexData = (int*)std::malloc((size_t)uvIndexCount * sizeof(int));
            if (!uvData || !uvIndexData){ if(uvData)std::free(uvData); if(uvIndexData)std::free(uvIndexData); uvData=0; uvIndexData=0; }
            if (uvData && uvIndexData)
            {
                char* ua = find_after(uvSec, "a:");
                char* ui = find_after(uvISec, "a:");
                for (int i=0;i<uvDataCount;i++) uvData[i] = (float)std::strtod(ua, &ua);
                for (int i=0;i<uvIndexCount;i++) uvIndexData[i] = (int)std::strtol(ui, &ui, 10);
            }
        }
    }

    char* pa = find_after(pSec, "a:"); if(!pa){ HuhluFBX_FreeMesh(out); std::free(data); return 0; }
    int tri=0; int face[8]; int fn=0; int faceStart=0;
    for (int i=0;i<polyCount;i++)
    {
        int idx = (int)std::strtol(pa, &pa, 10);
        int end = idx < 0; if (end) idx = -idx - 1;
        face[fn++] = idx;
        if (end)
        {
            for (int k=1;k+1<fn;k++)
            {
                out->indices[tri] = face[0];
                if (uvData && uvIndexData)
                {
                    int i0 = uvIndexData[faceStart + 0];
                    int i1 = uvIndexData[faceStart + k];
                    int i2 = uvIndexData[faceStart + k + 1];
                    if (i0 >= 0 && i0*2+1 < uvDataCount){ out->uvs[tri*2+0]=uvData[i0*2+0]; out->uvs[tri*2+1]=1.0f-uvData[i0*2+1]; }
                }
                if (nrmData){ int ni = (nrmIndexData && faceStart < nrmIndexCount)?nrmIndexData[faceStart+0]:face[0]; if(ni>=0 && ni*3+2<nrmDataCount){ out->normals[(tri)*3+0]=nrmData[ni*3+0]; out->normals[(tri)*3+1]=nrmData[ni*3+1]; out->normals[(tri)*3+2]=nrmData[ni*3+2]; }}
                if (nrmData){ int ni = (nrmIndexData && faceStart+k < nrmIndexCount)?nrmIndexData[faceStart+k]:face[k]; if(ni>=0 && ni*3+2<nrmDataCount){ out->normals[(tri)*3+0]=nrmData[ni*3+0]; out->normals[(tri)*3+1]=nrmData[ni*3+1]; out->normals[(tri)*3+2]=nrmData[ni*3+2]; }}
                if (nrmData){ int ni = (nrmIndexData && faceStart+k+1 < nrmIndexCount)?nrmIndexData[faceStart+k+1]:face[k+1]; if(ni>=0 && ni*3+2<nrmDataCount){ out->normals[(tri)*3+0]=nrmData[ni*3+0]; out->normals[(tri)*3+1]=nrmData[ni*3+1]; out->normals[(tri)*3+2]=nrmData[ni*3+2]; }}
                tri++;
                out->indices[tri] = face[k];
                if (uvData && uvIndexData)
                {
                    int i1 = uvIndexData[faceStart + k];
                    if (i1 >= 0 && i1*2+1 < uvDataCount){ out->uvs[tri*2+0]=uvData[i1*2+0]; out->uvs[tri*2+1]=1.0f-uvData[i1*2+1]; }
                }
                tri++;
                out->indices[tri] = face[k+1];
                if (uvData && uvIndexData)
                {
                    int i2 = uvIndexData[faceStart + k + 1];
                    if (i2 >= 0 && i2*2+1 < uvDataCount){ out->uvs[tri*2+0]=uvData[i2*2+0]; out->uvs[tri*2+1]=1.0f-uvData[i2*2+1]; }
                }
                tri++;
            }
            faceStart += fn;
            fn=0;
        }
    }

    char* tex = std::strstr(data, "RelativeFilename");
    if (tex)
    {
        char* q1 = std::strchr(tex, '"');
        if (q1){ q1 = std::strchr(q1+1, '"'); }
        if (q1){ char* q2 = std::strchr(q1+1, '"'); if (q2){ int n=(int)(q2-(q1+1)); if(n>0 && n<(int)sizeof(out->texturePath)){ std::memcpy(out->texturePath,q1+1,(size_t)n); out->texturePath[n]=0; } } }
    }

    out->vertexCount = vcount;
    out->indexCount = tri;

    if (uvData) std::free(uvData);
    if (uvIndexData) std::free(uvIndexData);
    if (nrmData) std::free(nrmData);
    if (nrmIndexData) std::free(nrmIndexData);
    std::free(data);
    return tri > 0 ? 1 : 0;
}
