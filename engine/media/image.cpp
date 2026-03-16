#include "image.h"

#include <windows.h>
#include <gdiplus.h>
#include <d3d9.h>
#include <cstdio>
#include <cstring>

#ifdef _MSC_VER
#pragma comment(lib, "gdiplus.lib")
#endif

using namespace Gdiplus;

namespace
{
    static ULONG_PTR g_gdiplusToken = 0;
    static char g_lastError[256] = "Image ready";

    static void Image_SetError(const char* text)
    {
        std::snprintf(g_lastError, sizeof(g_lastError), "%s", text ? text : "Unknown image error");
    }

    static int Image_PathToWide(const char* path, WCHAR* outWide, int outCount)
    {
        if (!path || !outWide || outCount <= 0)
            return 0;

        return MultiByteToWideChar(CP_ACP, 0, path, -1, outWide, outCount) > 0;
    }
}

int Image_Initialize()
{
    if (g_gdiplusToken != 0)
        return 1;

    GdiplusStartupInput input;
    if (GdiplusStartup(&g_gdiplusToken, &input, NULL) != Ok)
    {
        g_gdiplusToken = 0;
        Image_SetError("GDI+ startup failed");
        return 0;
    }

    Image_SetError("Image ready");
    return 1;
}

void Image_Shutdown()
{
    if (g_gdiplusToken != 0)
    {
        GdiplusShutdown(g_gdiplusToken);
        g_gdiplusToken = 0;
    }

    Image_SetError("Image offline");
}

int Image_LoadPNG(IDirect3DDevice9* device, const char* path, IXImage* outImage)
{
    WCHAR widePath[MAX_PATH];
    Bitmap* bitmap;
    UINT width;
    UINT height;
    IDirect3DTexture9* texture = NULL;
    HRESULT hr;
    Rect rect;
    BitmapData data;
    D3DLOCKED_RECT locked;
    UINT y;

    if (!device || !path || !outImage)
    {
        Image_SetError("Invalid image arguments");
        return 0;
    }

    Image_Release(outImage);

    if (!Image_Initialize())
        return 0;

    if (!Image_PathToWide(path, widePath, MAX_PATH))
    {
        Image_SetError("Wide path conversion failed");
        return 0;
    }

    bitmap = new Bitmap(widePath, FALSE);
    if (!bitmap || bitmap->GetLastStatus() != Ok)
    {
        delete bitmap;
        Image_SetError("PNG load failed");
        return 0;
    }

    width = bitmap->GetWidth();
    height = bitmap->GetHeight();
    if (width == 0 || height == 0)
    {
        delete bitmap;
        Image_SetError("PNG is empty");
        return 0;
    }

    hr = device->CreateTexture(
        width,
        height,
        1,
        0,
        D3DFMT_A8R8G8B8,
        D3DPOOL_MANAGED,
        &texture,
        NULL);

    if (FAILED(hr) || !texture)
    {
        delete bitmap;
        Image_SetError("Texture create failed");
        return 0;
    }

    rect = Rect(0, 0, (INT)width, (INT)height);
    if (bitmap->LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, &data) != Ok)
    {
        texture->Release();
        delete bitmap;
        Image_SetError("PNG lock failed");
        return 0;
    }

    hr = texture->LockRect(0, &locked, NULL, 0);
    if (FAILED(hr))
    {
        bitmap->UnlockBits(&data);
        texture->Release();
        delete bitmap;
        Image_SetError("Texture lock failed");
        return 0;
    }

    for (y = 0; y < height; ++y)
    {
        const unsigned char* src = (const unsigned char*)data.Scan0 + y * data.Stride;
        unsigned int* dst = (unsigned int*)((unsigned char*)locked.pBits + y * locked.Pitch);
        UINT x;
        for (x = 0; x < width; ++x)
        {
            unsigned char b = src[x * 4 + 0];
            unsigned char g = src[x * 4 + 1];
            unsigned char r = src[x * 4 + 2];
            unsigned char a = src[x * 4 + 3];
            dst[x] = ((unsigned int)a << 24) | ((unsigned int)r << 16) | ((unsigned int)g << 8) | (unsigned int)b;
        }
    }

    texture->UnlockRect(0);
    bitmap->UnlockBits(&data);
    delete bitmap;

    outImage->texture = texture;
    outImage->width = (int)width;
    outImage->height = (int)height;
    outImage->loaded = 1;
    std::snprintf(outImage->path, sizeof(outImage->path), "%s", path);

    Image_SetError("PNG loaded");
    return 1;
}

void Image_Release(IXImage* image)
{
    if (!image)
        return;

    if (image->texture)
    {
        image->texture->Release();
        image->texture = NULL;
    }

    image->width = 0;
    image->height = 0;
    image->loaded = 0;
    image->path[0] = '\0';
}

int Image_IsLoaded(const IXImage* image)
{
    return image && image->loaded && image->texture;
}

int Image_GetWidth(const IXImage* image)
{
    return image ? image->width : 0;
}

int Image_GetHeight(const IXImage* image)
{
    return image ? image->height : 0;
}

const char* Image_GetPath(const IXImage* image)
{
    return image ? image->path : "";
}

const char* Image_GetLastError()
{
    return g_lastError;
}
