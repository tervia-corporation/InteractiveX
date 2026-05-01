#include "editor.h"
#include <cstdio>
#include <cstring>

struct IXEditorObject
{
    int id;
    int parentId;
    int textureId;
    char name[64];
    float px, py, pz;
    float sx, sy, sz;
};

struct IXEditorTexture
{
    int id;
    char name[64];
    char filePath[260];
};

static int g_ready = 0;
static int g_nextId = 1;
static int g_nextTextureId = 1;
static int g_selectedObjectId = -1;
static IXEditorObject g_objects[256];
static int g_objectCount = 0;
static IXEditorTexture g_textures[256];
static int g_textureCount = 0;

static IXEditorObject* FindObject(int objectId)
{
    for (int i = 0; i < g_objectCount; ++i)
        if (g_objects[i].id == objectId) return &g_objects[i];
    return NULL;
}

static IXEditorTexture* FindTexture(int textureId)
{
    for (int i = 0; i < g_textureCount; ++i)
        if (g_textures[i].id == textureId) return &g_textures[i];
    return NULL;
}

static int HasTextureExtension(const char* filePath)
{
    if (!filePath) return 0;
    const char* ext = std::strrchr(filePath, '.');
    if (!ext) return 0;

    return std::strcmp(ext, ".png") == 0 ||
           std::strcmp(ext, ".jpg") == 0 ||
           std::strcmp(ext, ".jpeg") == 0 ||
           std::strcmp(ext, ".bmp") == 0 ||
           std::strcmp(ext, ".tga") == 0;
}

static void AddObject(const char* name, int parentId)
{
    if (g_objectCount >= 256) return;

    IXEditorObject& o = g_objects[g_objectCount++];
    o.id = g_nextId++;
    o.parentId = parentId;
    o.textureId = 0;
    std::snprintf(o.name, sizeof(o.name), "%s", (name && name[0]) ? name : "Cube");
    o.px = o.py = o.pz = 0.0f;
    o.sx = o.sy = o.sz = 1.0f;

    std::printf("[Editor] Added object #%d '%s' (parent=%d)\n", o.id, o.name, o.parentId);
}

int IXEditor_Initialize()
{
    g_ready = 1;
    g_nextId = 1;
    g_nextTextureId = 1;
    g_selectedObjectId = -1;
    g_objectCount = 0;
    g_textureCount = 0;

    std::printf("InteractiveX Editor initialized\n");

    AddObject("Scene", -1);
    AddObject("Cube_A", 1);
    AddObject("Cube_B", 1);
    g_selectedObjectId = 2;
    return 1;
}

void IXEditor_Shutdown()
{
    if (!g_ready) return;
    std::printf("InteractiveX Editor shutdown\n");
    g_ready = 0;
}

void IXEditor_ButtonAddCube(const char* name) { if (g_ready) AddObject(name, 1); }

void IXEditor_ButtonMoveObject(int objectId, float dx, float dy, float dz)
{
    if (!g_ready) return;
    IXEditorObject* o = FindObject(objectId);
    if (!o) return;
    o->px += dx; o->py += dy; o->pz += dz;
    std::printf("[Editor] Move #%d -> pos(%.2f, %.2f, %.2f)\n", o->id, o->px, o->py, o->pz);
}

void IXEditor_ButtonResizeObject(int objectId, float sx, float sy, float sz)
{
    if (!g_ready) return;
    IXEditorObject* o = FindObject(objectId);
    if (!o) return;
    if (sx < 0.01f) sx = 0.01f; if (sy < 0.01f) sy = 0.01f; if (sz < 0.01f) sz = 0.01f;
    o->sx = sx; o->sy = sy; o->sz = sz;
    std::printf("[Editor] Resize #%d -> scale(%.2f, %.2f, %.2f)\n", o->id, o->sx, o->sy, o->sz);
}

void IXEditor_ButtonRenameObject(int objectId, const char* newName)
{
    if (!g_ready || !newName || !newName[0]) return;
    IXEditorObject* o = FindObject(objectId);
    if (!o) return;
    std::snprintf(o->name, sizeof(o->name), "%s", newName);
    std::printf("[Editor] Rename #%d -> '%s'\n", o->id, o->name);
}

void IXEditor_ButtonDeleteObject(int objectId)
{
    if (!g_ready || objectId == 1) return; // preserve root scene
    for (int i = 0; i < g_objectCount; ++i)
    {
        if (g_objects[i].id == objectId)
        {
            for (int j = i; j < g_objectCount - 1; ++j) g_objects[j] = g_objects[j + 1];
            --g_objectCount;
            if (g_selectedObjectId == objectId) g_selectedObjectId = -1;
            std::printf("[Editor] Deleted object #%d\n", objectId);
            return;
        }
    }
}

void IXEditor_ButtonSelectObject(int objectId)
{
    if (!g_ready) return;
    if (FindObject(objectId))
    {
        g_selectedObjectId = objectId;
        std::printf("[Editor] Selected object #%d\n", g_selectedObjectId);
    }
}

int IXEditor_ButtonImportTexture(const char* filePath, const char* textureName)
{
    if (!g_ready || !filePath || !filePath[0]) return 0;
    if (g_textureCount >= 256) return 0;
    if (!HasTextureExtension(filePath)) return 0;

    IXEditorTexture& tex = g_textures[g_textureCount++];
    tex.id = g_nextTextureId++;
    std::snprintf(tex.filePath, sizeof(tex.filePath), "%s", filePath);
    std::snprintf(tex.name, sizeof(tex.name), "%s", (textureName && textureName[0]) ? textureName : "Texture");
    std::printf("[Editor] Imported texture #%d '%s' from '%s'\n", tex.id, tex.name, tex.filePath);
    return tex.id;
}

void IXEditor_ButtonAssignTextureToObject(int objectId, int textureId)
{
    if (!g_ready) return;
    IXEditorObject* o = FindObject(objectId);
    IXEditorTexture* t = FindTexture(textureId);
    if (!o || !t) return;
    o->textureId = t->id;
    std::printf("[Editor] Assigned texture #%d to object #%d\n", t->id, o->id);
}

void IXEditor_DrawTextureLibrary()
{
    if (!g_ready) return;
    std::printf("\n=== Texture Library ===\n");
    if (g_textureCount == 0) { std::printf("(empty)\n"); return; }
    for (int i = 0; i < g_textureCount; ++i)
        std::printf("[%d] %s -> %s\n", g_textures[i].id, g_textures[i].name, g_textures[i].filePath);
}

void IXEditor_DrawHierarchy()
{
    if (!g_ready) return;
    std::printf("\n=== Hierarchy ===\n");
    for (int i = 0; i < g_objectCount; ++i)
    {
        const IXEditorObject& o = g_objects[i];
        const char* marker = (o.id == g_selectedObjectId) ? "*" : " ";
        if (o.parentId < 0)
            std::printf("%s[%d] %s\n", marker, o.id, o.name);
        else
            std::printf("%s └─ [%d] %s (parent=%d)\n", marker, o.id, o.name, o.parentId);
    }
}

void IXEditor_DrawInspector(int objectId)
{
    if (!g_ready) return;
    IXEditorObject* o = FindObject(objectId);
    if (!o) return;
    std::printf("\n=== Inspector: #%d '%s' ===\n", o->id, o->name);
    std::printf("Position: (%.2f, %.2f, %.2f)\n", o->px, o->py, o->pz);
    std::printf("Scale:    (%.2f, %.2f, %.2f)\n", o->sx, o->sy, o->sz);
    std::printf("Texture:  %d\n", o->textureId);
}

void IXEditor_Update()
{
    if (!g_ready) return;
    IXEditor_DrawHierarchy();
    IXEditor_DrawTextureLibrary();
    if (g_selectedObjectId > 0) IXEditor_DrawInspector(g_selectedObjectId);
}
