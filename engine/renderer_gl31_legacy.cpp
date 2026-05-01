#include "renderer_gl31_legacy.h"

#ifndef IX_ENABLE_OPENGL31
#define IX_ENABLE_OPENGL31 0
#endif

int Renderer_GL31Legacy_IsCompiled()
{
#if IX_ENABLE_OPENGL31
    return 1;
#else
    return 0;
#endif
}
