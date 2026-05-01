#include "huhlu_renderer.h"
#if !IX_ENABLE_DX9
extern "C" int HuhluRenderer_Initialize(){return 1;}
extern "C" void HuhluRenderer_Shutdown(){}
extern "C" void HuhluRenderer_DrawCube(const IXHuhluCamera* camera){(void)camera;}
#if IX_ENABLE_DX10
extern "C" const char* HuhluRenderer_GetBackendName(){return "DX10";}
#elif IX_ENABLE_DX11
extern "C" const char* HuhluRenderer_GetBackendName(){return "DX11";}
#elif IX_ENABLE_DX12
extern "C" const char* HuhluRenderer_GetBackendName(){return "DX12";}
#elif IX_ENABLE_VULKAN
extern "C" const char* HuhluRenderer_GetBackendName(){return "Vulkan";}
#elif IX_ENABLE_OPENGL43
extern "C" const char* HuhluRenderer_GetBackendName(){return "OpenGL 4.3";}
#elif IX_ENABLE_OPENGL31
extern "C" const char* HuhluRenderer_GetBackendName(){return "OpenGL 3.1";}
#else
extern "C" const char* HuhluRenderer_GetBackendName(){return "Unknown";}
#endif
#endif
