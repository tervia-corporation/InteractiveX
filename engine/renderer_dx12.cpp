#include "renderer_dx12.h"

#ifndef IX_ENABLE_DX12
#define IX_ENABLE_DX12 0
#endif

int Renderer_DX12_IsCompiled()
{
#if IX_ENABLE_DX12
    return 1;
#else
    return 0;
#endif
}
