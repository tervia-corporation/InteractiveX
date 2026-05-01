#include "huhlu_renderer.h"
#include "huhlu_backend_common.h"
#if IX_ENABLE_VULKAN
extern "C" int HuhluRenderer_Initialize(){return 1;}
extern "C" void HuhluRenderer_Shutdown(){}
extern "C" void HuhluRenderer_DrawCube(const IXHuhluCamera* camera){ Huhlu_DrawSoftwareCube(camera); }
extern "C" const char* HuhluRenderer_GetBackendName(){return "vulkan";}
#endif
