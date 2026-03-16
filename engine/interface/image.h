#ifndef IX_IMAGE_H
#define IX_IMAGE_H

#include <windows.h>

struct IDirect3DDevice9;
struct IDirect3DTexture9;

enum IXImageFormat
{
    IX_IMAGE_UNKNOWN = 0,
    IX_IMAGE_PNG,
    IX_IMAGE_BMP
};

struct IXImage
{
    IDirect3DTexture9* texture;
    int width;
    int height;
    IXImageFormat format;
};

bool Image_Initialize();
void Image_Shutdown();
bool Image_LoadFromFile(IDirect3DDevice9* device, const char* path, IXImage* outImage);
void Image_Release(IXImage* image);
const char* Image_GetLastError();

#endif
