#include "renderer_gl43.h"

#ifndef IX_ENABLE_OPENGL43
#define IX_ENABLE_OPENGL43 0
#endif

int Renderer_GL43_IsCompiled()
{
#if IX_ENABLE_OPENGL43
    return 1;
#else
    return 0;
#endif
}
