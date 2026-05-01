#include "renderer_dx11.h"

#ifndef IX_ENABLE_DX11
#define IX_ENABLE_DX11 0
#endif

int Renderer_DX11_IsCompiled()
{
#if IX_ENABLE_DX11
    return 1;
#else
    return 0;
#endif
}
