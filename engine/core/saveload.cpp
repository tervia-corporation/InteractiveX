#include "saveload.h"
#include <windows.h>
#include <shlobj.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>

// ---------------------------------------------------------------------------
//  Internos
// ---------------------------------------------------------------------------

#define IX_SAVE_MAX_ENTRIES  256
#define IX_SAVE_MAX_KEY       64
#define IX_SAVE_MAX_PATH     512
#define IX_SAVE_EXT          ".ixsav"

static char g_basePath[IX_SAVE_MAX_PATH] = "";

// Garante que o diretorio existe (cria recursivamente se necessario).
static void EnsureDir(const char* path)
{
    // SHCreateDirectoryExA cria toda a hierarquia e nao falha se ja existe.
    SHCreateDirectoryExA(NULL, path, NULL);
}

// Monta "<basePath>\<filename>[.ixsav]" em out.
static void BuildFilePath(char* out, unsigned int outSize, const char* filename)
{
    char name[IX_SAVE_MAX_PATH];
    // Adiciona extensao se o filename nao tiver.
    if (strstr(filename, ".ixsav") != NULL)
        _snprintf_s(name, sizeof(name), _TRUNCATE, "%s", filename);
    else
        _snprintf_s(name, sizeof(name), _TRUNCATE, "%s%s", filename, IX_SAVE_EXT);

    if (g_basePath[0] != '\0')
        _snprintf_s(out, outSize, _TRUNCATE, "%s\\%s", g_basePath, name);
    else
        _snprintf_s(out, outSize, _TRUNCATE, "%s", name);
}

// ---------------------------------------------------------------------------
//  Configuracao de caminho
// ---------------------------------------------------------------------------

void SaveLoad_SetBasePath(int root, const char* subdir)
{
    char base[IX_SAVE_MAX_PATH] = "";

    switch (root)
    {
    case IX_SAVE_PATH_APPDATA:
    {
        const char* appdata = getenv("APPDATA");
        if (appdata)
            _snprintf_s(base, sizeof(base), _TRUNCATE, "%s", appdata);
        break;
    }
    case IX_SAVE_PATH_DOCUMENTS:
    {
        // SHGetFolderPathA para CSIDL_PERSONAL = Documentos
        SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, base);
        break;
    }
    case IX_SAVE_PATH_EXE:
    default:
    {
        // Diretorio do executavel
        GetModuleFileNameA(NULL, base, (DWORD)sizeof(base));
        char* last = strrchr(base, '\\');
        if (last) *last = '\0'; else base[0] = '\0';
        break;
    }
    }

    if (subdir && subdir[0] != '\0')
        _snprintf_s(g_basePath, sizeof(g_basePath), _TRUNCATE, "%s\\%s", base, subdir);
    else
        _snprintf_s(g_basePath, sizeof(g_basePath), _TRUNCATE, "%s", base);

    EnsureDir(g_basePath);
}

void SaveLoad_SetBasePathAbsolute(const char* absolutePath)
{
    _snprintf_s(g_basePath, sizeof(g_basePath), _TRUNCATE, "%s", absolutePath ? absolutePath : "");
    if (g_basePath[0] != '\0')
        EnsureDir(g_basePath);
}

const char* SaveLoad_GetBasePath()
{
    return g_basePath;
}

// ---------------------------------------------------------------------------
//  Writer
// ---------------------------------------------------------------------------

typedef struct IXSaveEntry
{
    char          key[IX_SAVE_MAX_KEY];
    unsigned char* data;
    unsigned int   len;
} IXSaveEntry;

struct IXSaveWriter
{
    unsigned int version;
    IXSaveEntry  entries[IX_SAVE_MAX_ENTRIES];
    unsigned int count;
};

IXSaveWriter* SaveLoad_WriterCreate(unsigned int version)
{
    IXSaveWriter* w = (IXSaveWriter*)calloc(1, sizeof(IXSaveWriter));
    if (w) w->version = version;
    return w;
}

void SaveLoad_WriterPutBytes(IXSaveWriter* w, const char* key, const void* data, unsigned int len)
{
    if (!w || !key || !data || len == 0) return;
    if (w->count >= IX_SAVE_MAX_ENTRIES) return;

    // Substitui entrada existente se a chave ja existe.
    for (unsigned int i = 0; i < w->count; i++)
    {
        if (strcmp(w->entries[i].key, key) == 0)
        {
            free(w->entries[i].data);
            w->entries[i].data = (unsigned char*)malloc(len);
            if (w->entries[i].data) { memcpy(w->entries[i].data, data, len); w->entries[i].len = len; }
            return;
        }
    }

    IXSaveEntry* e = &w->entries[w->count++];
    _snprintf_s(e->key, sizeof(e->key), _TRUNCATE, "%s", key);
    e->data = (unsigned char*)malloc(len);
    if (e->data) { memcpy(e->data, data, len); e->len = len; }
}

void SaveLoad_WriterPutInt(IXSaveWriter* w, const char* key, int value)
{
    SaveLoad_WriterPutBytes(w, key, &value, sizeof(int));
}

void SaveLoad_WriterPutFloat(IXSaveWriter* w, const char* key, float value)
{
    SaveLoad_WriterPutBytes(w, key, &value, sizeof(float));
}

void SaveLoad_WriterPutStr(IXSaveWriter* w, const char* key, const char* str)
{
    if (!str) return;
    unsigned int len = (unsigned int)strlen(str) + 1; // inclui nulo
    SaveLoad_WriterPutBytes(w, key, str, len);
}

int SaveLoad_WriterSave(IXSaveWriter* w, const char* filename)
{
    if (!w || !filename) return 0;

    char path[IX_SAVE_MAX_PATH];
    BuildFilePath(path, sizeof(path), filename);

    FILE* f = NULL;
    if (fopen_s(&f, path, "wb") != 0 || !f) return 0;

    // Header
    unsigned int magic   = IX_SAVE_MAGIC;
    unsigned int version = w->version;
    unsigned int count   = w->count;
    fwrite(&magic,   4, 1, f);
    fwrite(&version, 4, 1, f);
    fwrite(&count,   4, 1, f);

    // Entradas
    for (unsigned int i = 0; i < w->count; i++)
    {
        IXSaveEntry* e = &w->entries[i];
        unsigned short keyLen = (unsigned short)strlen(e->key);
        fwrite(&keyLen,  2, 1, f);
        fwrite(e->key,   1, keyLen, f);
        fwrite(&e->len,  4, 1, f);
        fwrite(e->data,  1, e->len, f);
    }

    fclose(f);
    return 1;
}

void SaveLoad_WriterDestroy(IXSaveWriter* w)
{
    if (!w) return;
    for (unsigned int i = 0; i < w->count; i++)
        free(w->entries[i].data);
    free(w);
}

// ---------------------------------------------------------------------------
//  Reader
// ---------------------------------------------------------------------------

struct IXSaveReader
{
    unsigned int version;
    IXSaveEntry  entries[IX_SAVE_MAX_ENTRIES];
    unsigned int count;
};

IXSaveReader* SaveLoad_ReaderOpen(const char* filename)
{
    if (!filename) return NULL;

    char path[IX_SAVE_MAX_PATH];
    BuildFilePath(path, sizeof(path), filename);

    FILE* f = NULL;
    if (fopen_s(&f, path, "rb") != 0 || !f) return NULL;

    unsigned int magic, version, count;
    if (fread(&magic,   4, 1, f) != 1 || magic != IX_SAVE_MAGIC) { fclose(f); return NULL; }
    if (fread(&version, 4, 1, f) != 1) { fclose(f); return NULL; }
    if (fread(&count,   4, 1, f) != 1 || count > IX_SAVE_MAX_ENTRIES) { fclose(f); return NULL; }

    IXSaveReader* r = (IXSaveReader*)calloc(1, sizeof(IXSaveReader));
    if (!r) { fclose(f); return NULL; }

    r->version = version;
    r->count   = 0;

    for (unsigned int i = 0; i < count; i++)
    {
        unsigned short keyLen = 0;
        if (fread(&keyLen, 2, 1, f) != 1 || keyLen >= IX_SAVE_MAX_KEY) { goto fail; }

        IXSaveEntry* e = &r->entries[r->count];
        if (fread(e->key, 1, keyLen, f) != keyLen) goto fail;
        e->key[keyLen] = '\0';

        unsigned int dataLen = 0;
        if (fread(&dataLen, 4, 1, f) != 1) goto fail;

        e->data = (unsigned char*)malloc(dataLen);
        if (!e->data) goto fail;
        if (fread(e->data, 1, dataLen, f) != dataLen) { free(e->data); goto fail; }
        e->len = dataLen;
        r->count++;
    }

    fclose(f);
    return r;

fail:
    for (unsigned int i = 0; i < r->count; i++) free(r->entries[i].data);
    free(r);
    fclose(f);
    return NULL;
}

unsigned int SaveLoad_ReaderGetVersion(IXSaveReader* r)
{
    return r ? r->version : 0;
}

unsigned int SaveLoad_ReaderGetCount(IXSaveReader* r)
{
    return r ? r->count : 0;
}

const void* SaveLoad_ReaderGetBytes(IXSaveReader* r, const char* key, unsigned int* outLen)
{
    if (!r || !key) return NULL;
    for (unsigned int i = 0; i < r->count; i++)
    {
        if (strcmp(r->entries[i].key, key) == 0)
        {
            if (outLen) *outLen = r->entries[i].len;
            return r->entries[i].data;
        }
    }
    return NULL;
}

int SaveLoad_ReaderGetInt(IXSaveReader* r, const char* key)
{
    unsigned int len = 0;
    const void* p = SaveLoad_ReaderGetBytes(r, key, &len);
    if (!p || len < sizeof(int)) return 0;
    int v; memcpy(&v, p, sizeof(int)); return v;
}

float SaveLoad_ReaderGetFloat(IXSaveReader* r, const char* key)
{
    unsigned int len = 0;
    const void* p = SaveLoad_ReaderGetBytes(r, key, &len);
    if (!p || len < sizeof(float)) return 0.0f;
    float v; memcpy(&v, p, sizeof(float)); return v;
}

int SaveLoad_ReaderGetStr(IXSaveReader* r, const char* key, char* buf, unsigned int bufLen)
{
    if (!buf || bufLen == 0) return 0;
    unsigned int len = 0;
    const void* p = SaveLoad_ReaderGetBytes(r, key, &len);
    if (!p) { buf[0] = '\0'; return 0; }
    unsigned int copy = len < bufLen ? len : bufLen - 1;
    memcpy(buf, p, copy);
    buf[copy] = '\0';
    return 1;
}

void SaveLoad_ReaderDestroy(IXSaveReader* r)
{
    if (!r) return;
    for (unsigned int i = 0; i < r->count; i++) free(r->entries[i].data);
    free(r);
}

// ---------------------------------------------------------------------------
//  Utilitarios
// ---------------------------------------------------------------------------

int SaveLoad_FileExists(const char* filename)
{
    if (!filename) return 0;
    char path[IX_SAVE_MAX_PATH];
    BuildFilePath(path, sizeof(path), filename);
    DWORD attr = GetFileAttributesA(path);
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

int SaveLoad_FileDelete(const char* filename)
{
    if (!filename) return 0;
    char path[IX_SAVE_MAX_PATH];
    BuildFilePath(path, sizeof(path), filename);
    return DeleteFileA(path) ? 1 : 0;
}
