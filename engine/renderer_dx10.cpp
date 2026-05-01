#include "renderer_dx10.h"

#ifndef IX_ENABLE_DX10
#define IX_ENABLE_DX10 0
#endif

int Renderer_DX10_IsCompiled()
{
#if IX_ENABLE_DX10
    return 1;
#else
    return 0;
#endif
}
