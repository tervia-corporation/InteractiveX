#include "huhlu_hierarchy.h"

void HuhluHierarchy_UpdateWorld(IXHuhluNode* n, int c)
{
    if(!n||c<=0) return;
    for(int i=0;i<c;i++){
        int p=n[i].parent;
        if(p>=0&&p<c){ n[i].wx=n[p].wx+n[i].tx; n[i].wy=n[p].wy+n[i].ty; n[i].wz=n[p].wz+n[i].tz; }
        else { n[i].wx=n[i].tx; n[i].wy=n[i].ty; n[i].wz=n[i].tz; }
    }
}
