#include "assets.h"

#include <windows.h>
#include <cstdio>
#include <cstring>

namespace
{
    static char g_basePath[MAX_PATH] = {0};
    static char g_assetsRoot[MAX_PATH] = {0};
    static char g_imagesRoot[MAX_PATH] = {0};
    static char g_audioRoot[MAX_PATH] = {0};

    static void CopyStringSafe(char* dst, int dstSize, const char* src)
    {
        if (!dst || dstSize <= 0)
            return;

        if (!src)
        {
            dst[0] = '\0';
            return;
        }

        std::snprintf(dst, (size_t)dstSize, "%s", src);
    }

    static void NormalizeSlashes(char* path)
    {
        int i;
        if (!path)
            return;

        for (i = 0; path[i] != '\0'; ++i)
        {
            if (path[i] == '/')
                path[i] = '\\';
        }
    }

    static void TrimTrailingSlash(char* path)
    {
        int len;
        if (!path)
            return;

        len = (int)std::strlen(path);
        while (len > 0 && (path[len - 1] == '\\' || path[len - 1] == '/'))
        {
            path[len - 1] = '\0';
            --len;
        }
    }

    static void RefreshDerivedPaths()
    {
        std::snprintf(g_assetsRoot, sizeof(g_assetsRoot), "%s\\assets", g_basePath);
        std::snprintf(g_imagesRoot, sizeof(g_imagesRoot), "%s\\images", g_assetsRoot);
        std::snprintf(g_audioRoot, sizeof(g_audioRoot), "%s\\audio", g_assetsRoot);
    }
}

int Assets_Initialize()
{
    char modulePath[MAX_PATH];
    char* slash;
    DWORD length = GetModuleFileNameA(NULL, modulePath, MAX_PATH);

    if (length == 0 || length >= MAX_PATH)
        return 0;

    NormalizeSlashes(modulePath);
    slash = std::strrchr(modulePath, '\\');
    if (!slash)
        return 0;

    *slash = '\0';
    Assets_SetBasePath(modulePath);
    return 1;
}

void Assets_Shutdown()
{
    g_basePath[0] = '\0';
    g_assetsRoot[0] = '\0';
    g_imagesRoot[0] = '\0';
    g_audioRoot[0] = '\0';
}

void Assets_SetBasePath(const char* basePath)
{
    CopyStringSafe(g_basePath, sizeof(g_basePath), basePath ? basePath : "");
    NormalizeSlashes(g_basePath);
    TrimTrailingSlash(g_basePath);
    RefreshDerivedPaths();
}

const char* Assets_GetBasePath()
{
    return g_basePath;
}

const char* Assets_GetAssetsRoot()
{
    return g_assetsRoot;
}

const char* Assets_GetImagesRoot()
{
    return g_imagesRoot;
}

const char* Assets_GetAudioRoot()
{
    return g_audioRoot;
}

int Assets_BuildPath(const char* relativePath, char* outPath, int outPathSize)
{
    if (!relativePath || !outPath || outPathSize <= 0)
        return 0;

    if (g_assetsRoot[0] == '\0')
        return 0;

    std::snprintf(outPath, (size_t)outPathSize, "%s\\%s", g_assetsRoot, relativePath);
    NormalizeSlashes(outPath);
    return 1;
}

int Assets_BuildImagePath(const char* fileName, char* outPath, int outPathSize)
{
    if (!fileName || !outPath || outPathSize <= 0)
        return 0;

    if (g_imagesRoot[0] == '\0')
        return 0;

    std::snprintf(outPath, (size_t)outPathSize, "%s\\%s", g_imagesRoot, fileName);
    NormalizeSlashes(outPath);
    return 1;
}

int Assets_BuildAudioPath(const char* fileName, char* outPath, int outPathSize)
{
    if (!fileName || !outPath || outPathSize <= 0)
        return 0;

    if (g_audioRoot[0] == '\0')
        return 0;

    std::snprintf(outPath, (size_t)outPathSize, "%s\\%s", g_audioRoot, fileName);
    NormalizeSlashes(outPath);
    return 1;
}

int Assets_FileExists(const char* fullPath)
{
    DWORD attributes;

    if (!fullPath || fullPath[0] == '\0')
        return 0;

    attributes = GetFileAttributesA(fullPath);
    if (attributes == INVALID_FILE_ATTRIBUTES)
        return 0;

    return (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

int Assets_ImageExists(const char* fileName)
{
    char path[MAX_PATH];

    if (!Assets_BuildImagePath(fileName, path, sizeof(path)))
        return 0;

    return Assets_FileExists(path);
}

int Assets_AudioExists(const char* fileName)
{
    char path[MAX_PATH];

    if (!Assets_BuildAudioPath(fileName, path, sizeof(path)))
        return 0;

    return Assets_FileExists(path);
}
