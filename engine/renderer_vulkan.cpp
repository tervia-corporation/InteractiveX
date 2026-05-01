#include "renderer_vulkan.h"

#ifndef IX_ENABLE_VULKAN
#define IX_ENABLE_VULKAN 0
#endif

int Renderer_Vulkan_IsCompiled()
{
#if IX_ENABLE_VULKAN
    return 1;
#else
    return 0;
#endif
}
