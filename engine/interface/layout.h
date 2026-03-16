#ifndef IX_LAYOUT_H
#define IX_LAYOUT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IXLayout
{
    int x;
    int y;
    int width;
    int height;

    int cursorX;
    int cursorY;

    int spacing;
    int padding;
} IXLayout;

void Layout_BeginVertical(IXLayout* layout, int x, int y, int width, int height, int spacing);
void Layout_SetPadding(IXLayout* layout, int padding);
void Layout_Reset(IXLayout* layout);
void Layout_GetContentRect(const IXLayout* layout, int* x, int* y, int* width, int* height);
void Layout_Next(IXLayout* layout, int itemHeight, int* x, int* y, int* width, int* height);
void Layout_Skip(IXLayout* layout, int amount);
int  Layout_GetCursorY(const IXLayout* layout);

#ifdef __cplusplus
}
#endif

#endif
