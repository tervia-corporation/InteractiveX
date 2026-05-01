#ifndef IX_STYLE_ISS_H
#define IX_STYLE_ISS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IXISSRule
{
    char selector[64];
    char property[64];
    char value[128];
} IXISSRule;

typedef struct IXISSRect
{
    int x, y, width, height;
} IXISSRect;

int  ISS_LoadFile(const char* path);
int  ISS_GetRuleCount();
const IXISSRule* ISS_GetRule(int index);
const char* ISS_GetLastError();
void ISS_Clear();

// Layout responsivo inspirado em CSS
void ISS_SetViewport(int width, int height);
int  ISS_ResolveLength(const char* value, int axisSize, int fontBase);
int  ISS_ComputeRect(const char* selector, IXISSRect* outRect);

int  ISS_ParseColor(const char* value, unsigned int* outARGB);
int  ISS_GetColor(const char* selector, const char* property, unsigned int* outARGB);
int  ISS_GetBackgroundImage(const char* selector, char* outPath, int outPathSize);
int  ISS_GetString(const char* selector, const char* property, char* outValue, int outValueSize);

typedef struct IXISSBoxStyle
{
    int width;
    int height;
    int weight;
    int alignX; // 0 start, 1 center, 2 end
    int alignY; // 0 start, 1 center, 2 end
    int borderWidth;
    unsigned int borderColor;
    int hasGradient;
    unsigned int gradientTop;
    unsigned int gradientBottom;
} IXISSBoxStyle;

int ISS_ComputeBoxStyle(const char* selector, IXISSBoxStyle* outStyle);
int ISS_LayoutStackVertical(const char** selectors, int count, IXISSRect container, int gap, IXISSRect* outRects);

#ifdef __cplusplus
}
#endif

#endif
