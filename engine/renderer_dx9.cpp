#include "renderer_dx9.h"

#ifndef IX_ENABLE_DX9
#define IX_ENABLE_DX9 0
#endif

int Renderer_DX9_IsCompiled()
{
#if IX_ENABLE_DX9
    return 1;
#else
    return 0;
#endif
}
