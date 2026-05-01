#ifndef IX_HUHLU_HIERARCHY_H
#define IX_HUHLU_HIERARCHY_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IXHuhluNode
{
    int parent; // -1 root
    float tx,ty,tz;
    float wx,wy,wz;
} IXHuhluNode;

void HuhluHierarchy_UpdateWorld(IXHuhluNode* nodes, int count);

#ifdef __cplusplus
}
#endif

#endif
