#include "layout.h"


static int g_refW = 1280;
static int g_refH = 720;
static int g_viewW = 1280;
static int g_viewH = 720;

void Layout_SetReferenceResolution(int width, int height)
{
    if (width > 0) g_refW = width;
    if (height > 0) g_refH = height;
}

void Layout_SetViewportResolution(int width, int height)
{
    if (width > 0) g_viewW = width;
    if (height > 0) g_viewH = height;
}

int Layout_ScaleX(int value)
{
    if (value <= 0 || g_refW <= 0) return value;
    return (value * g_viewW) / g_refW;
}

int Layout_ScaleY(int value)
{
    if (value <= 0 || g_refH <= 0) return value;
    return (value * g_viewH) / g_refH;
}

void Layout_BeginVerticalScaled(IXLayout* layout, int x, int y, int width, int height, int spacing)
{
    Layout_BeginVertical(layout,
        Layout_ScaleX(x),
        Layout_ScaleY(y),
        Layout_ScaleX(width),
        Layout_ScaleY(height),
        Layout_ScaleY(spacing));
}

static int Layout_ClampNonNegative(int value)
{
    return value < 0 ? 0 : value;
}

void Layout_BeginVertical(IXLayout* layout, int x, int y, int width, int height, int spacing)
{
    if (!layout)
        return;

    layout->x = x;
    layout->y = y;
    layout->width = Layout_ClampNonNegative(width);
    layout->height = Layout_ClampNonNegative(height);
    layout->spacing = Layout_ClampNonNegative(spacing);
    layout->padding = 0;
    layout->cursorX = x;
    layout->cursorY = y;
}

void Layout_SetPadding(IXLayout* layout, int padding)
{
    if (!layout)
        return;

    layout->padding = Layout_ClampNonNegative(padding);
    Layout_Reset(layout);
}

void Layout_Reset(IXLayout* layout)
{
    if (!layout)
        return;

    layout->cursorX = layout->x + layout->padding;
    layout->cursorY = layout->y + layout->padding;
}

void Layout_GetContentRect(const IXLayout* layout, int* x, int* y, int* width, int* height)
{
    int contentWidth;
    int contentHeight;

    if (!layout)
    {
        if (x) *x = 0;
        if (y) *y = 0;
        if (width) *width = 0;
        if (height) *height = 0;
        return;
    }

    contentWidth = layout->width - (layout->padding * 2);
    contentHeight = layout->height - (layout->padding * 2);
    if (contentWidth < 0) contentWidth = 0;
    if (contentHeight < 0) contentHeight = 0;

    if (x) *x = layout->x + layout->padding;
    if (y) *y = layout->y + layout->padding;
    if (width) *width = contentWidth;
    if (height) *height = contentHeight;
}

void Layout_Next(IXLayout* layout, int itemHeight, int* x, int* y, int* width, int* height)
{
    int contentX;
    int contentY;
    int contentWidth;
    int contentHeight;
    int finalHeight;

    if (!layout)
    {
        if (x) *x = 0;
        if (y) *y = 0;
        if (width) *width = 0;
        if (height) *height = 0;
        return;
    }

    Layout_GetContentRect(layout, &contentX, &contentY, &contentWidth, &contentHeight);
    finalHeight = Layout_ClampNonNegative(itemHeight);

    if (x) *x = contentX;
    if (y) *y = layout->cursorY;
    if (width) *width = contentWidth;
    if (height) *height = finalHeight;

    layout->cursorY += finalHeight + layout->spacing;
}

void Layout_Skip(IXLayout* layout, int amount)
{
    if (!layout)
        return;

    layout->cursorY += Layout_ClampNonNegative(amount);
}

int Layout_GetCursorY(const IXLayout* layout)
{
    if (!layout)
        return 0;

    return layout->cursorY;
}
