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

static ULONG_PTR g_gdiplusToken = 0;
static char g_imageLastError[256] = "Ready";

static void Image_SetError(const char* text)
{
    if (!text) text = "Unknown";
    std::snprintf(g_imageLastError, sizeof(g_imageLastError), "%s", text);
}

bool Image_Initialize()
{
    if (g_gdiplusToken != 0)
        return true;

    GdiplusStartupInput input;
    if (GdiplusStartup(&g_gdiplusToken, &input, NULL) != Ok)
    {
        Image_SetError("GDI+ startup failed");
        g_gdiplusToken = 0;
        return false;
    }

    Image_SetError("Ready");
    return true;
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

static bool Image_PathToWide(const char* path, WCHAR* outWide, int outCount)
{
    if (!path || !outWide || outCount <= 0)
        return false;

    int result = MultiByteToWideChar(CP_ACP, 0, path, -1, outWide, outCount);
    return result > 0;
}

bool Image_LoadFromFile(IDirect3DDevice9* device, const char* path, IXImage* outImage)
{
    if (!device || !path || !outImage)
    {
        Image_SetError("Invalid image arguments");
        return false;
    }

    outImage->texture = NULL;
    outImage->width = 0;
    outImage->height = 0;
    outImage->format = IX_IMAGE_UNKNOWN;

    if (!Image_Initialize())
        return false;

    WCHAR widePath[MAX_PATH];
    if (!Image_PathToWide(path, widePath, MAX_PATH))
    {
        Image_SetError("Wide path conversion failed");
        return false;
    }

    Bitmap bitmap(widePath, FALSE);
    if (bitmap.GetLastStatus() != Ok)
    {
        Image_SetError("Bitmap load failed");
        return false;
    }

    UINT width = bitmap.GetWidth();
    UINT height = bitmap.GetHeight();
    if (width == 0 || height == 0)
    {
        Image_SetError("Empty image");
        return false;
    }

    IDirect3DTexture9* texture = NULL;
    HRESULT hr = device->CreateTexture(
        (UINT)width,
        (UINT)height,
        1,
        0,
        D3DFMT_A8R8G8B8,
        D3DPOOL_MANAGED,
        &texture,
        NULL
    );

    if (FAILED(hr) || !texture)
    {
        Image_SetError("Texture create failed");
        return false;
    }

    Rect rect(0, 0, (INT)width, (INT)height);
    BitmapData data;
    if (bitmap.LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, &data) != Ok)
    {
        texture->Release();
        Image_SetError("Bitmap lock failed");
        return false;
    }

    D3DLOCKED_RECT locked;
    hr = texture->LockRect(0, &locked, NULL, 0);
    if (FAILED(hr))
    {
        bitmap.UnlockBits(&data);
        texture->Release();
        Image_SetError("Texture lock failed");
        return false;
    }

    for (UINT y = 0; y < height; ++y)
    {
        const unsigned char* src = (const unsigned char*)data.Scan0 + y * data.Stride;
        unsigned int* dst = (unsigned int*)((unsigned char*)locked.pBits + y * locked.Pitch);
        for (UINT x = 0; x < width; ++x)
        {
            unsigned char b = src[x * 4 + 0];
            unsigned char g = src[x * 4 + 1];
            unsigned char r = src[x * 4 + 2];
            unsigned char a = src[x * 4 + 3];
            dst[x] = ((unsigned int)a << 24) | ((unsigned int)r << 16) | ((unsigned int)g << 8) | (unsigned int)b;
        }
    }

    texture->UnlockRect(0);
    bitmap.UnlockBits(&data);

    outImage->texture = texture;
    outImage->width = (int)width;
    outImage->height = (int)height;

    const char* ext = std::strrchr(path, '.');
    if (ext && _stricmp(ext, ".png") == 0)
        outImage->format = IX_IMAGE_PNG;
    else if (ext && _stricmp(ext, ".bmp") == 0)
        outImage->format = IX_IMAGE_BMP;
    else
        outImage->format = IX_IMAGE_UNKNOWN;

    Image_SetError("Image loaded");
    return true;
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
    image->format = IX_IMAGE_UNKNOWN;
}

const char* Image_GetLastError()
{
    return g_imageLastError;
}
