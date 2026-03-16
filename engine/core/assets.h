#ifndef IX_CORE_ASSETS_H
#define IX_CORE_ASSETS_H

int Assets_Initialize();
void Assets_Shutdown();

void Assets_SetBasePath(const char* basePath);
const char* Assets_GetBasePath();
const char* Assets_GetAssetsRoot();
const char* Assets_GetImagesRoot();
const char* Assets_GetAudioRoot();

int Assets_BuildPath(const char* relativePath, char* outPath, int outPathSize);
int Assets_BuildImagePath(const char* fileName, char* outPath, int outPathSize);
int Assets_BuildAudioPath(const char* fileName, char* outPath, int outPathSize);

int Assets_FileExists(const char* fullPath);
int Assets_ImageExists(const char* fileName);
int Assets_AudioExists(const char* fileName);

#endif
