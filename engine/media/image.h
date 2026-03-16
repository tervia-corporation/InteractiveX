#ifndef IX_MEDIA_IMAGE_H
#define IX_MEDIA_IMAGE_H

#include <windows.h>

struct IDirect3DDevice9;
struct IDirect3DTexture9;

struct IXImage
{
    IDirect3DTexture9* texture;
    int width;
    int height;
    int loaded;
    char path[MAX_PATH];
};

int  Image_Initialize();
void Image_Shutdown();
int  Image_LoadPNG(IDirect3DDevice9* device, const char* path, IXImage* outImage);
void Image_Release(IXImage* image);
int  Image_IsLoaded(const IXImage* image);
int  Image_GetWidth(const IXImage* image);
int  Image_GetHeight(const IXImage* image);
const char* Image_GetPath(const IXImage* image);
const char* Image_GetLastError();

#endif
