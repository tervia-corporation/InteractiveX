#include "saveload_test.h"
#include "../renderer.h"
#include "../core/input.h"
#include "../core/saveload.h"
#include "../interface/button.h"
#include "../interface/layout.h"
#include <cstdio>
#include <cstring>

// ---------------------------------------------------------------------------
//  Save/Load Test
//
//  Demonstra o sistema IXSaveWriter / IXSaveReader com tres destinos:
//    - Pasta do executavel  (IX_SAVE_PATH_EXE)
//    - %APPDATA%\InteractiveX
//    - Documentos\InteractiveX\saves
//
//  O slot de teste grava: score (int), speed (float), name (str).
// ---------------------------------------------------------------------------

#define TEST_FILENAME   "debug_slot"
#define TEST_VERSION    1u
#define TEST_APPNAME    "InteractiveX"

namespace
{
    // Botoes
    static IXButton g_btnSaveExe;
    static IXButton g_btnSaveAppdata;
    static IXButton g_btnSaveDocuments;
    static IXButton g_btnLoad;
    static IXButton g_btnDelete;
    static const IXButton* g_buttons[5];

    static int  g_returnToMenu = 0;
    static char g_statusLine[256] = "Save/Load test ready.";
    static char g_helpLine[256]   = "ESC menu";

    // Dados de exibicao do ultimo load
    static int   g_loadedScore  = 0;
    static float g_loadedSpeed  = 0.0f;
    static char  g_loadedName[64] = "";
    static int   g_hasLoaded    = 0;

    // Qual destino esta ativo atualmente
    static int   g_currentRoot  = -1;
    static char  g_currentSubdir[128] = "";

    struct SLLayoutCache { int valid, x, y, width, height; };
    static SLLayoutCache g_layoutCache = {0, 0, 0, 0, 0};

    static void RefreshLayout()
    {
        IXLayout layout;
        int x, y, width, height;
        Renderer_GetOverlayContentRect(&x, &y, &width, &height);
        g_layoutCache = { 1, x, y, width, height };

        Layout_BeginVertical(&layout, x, y + 88, width, height - 88, 10);
        Layout_Next(&layout, 36, &x, &y, &width, &height); Button_SetBounds(&g_btnSaveExe,       x, y, width, height);
        Layout_Next(&layout, 36, &x, &y, &width, &height); Button_SetBounds(&g_btnSaveAppdata,   x, y, width, height);
        Layout_Next(&layout, 36, &x, &y, &width, &height); Button_SetBounds(&g_btnSaveDocuments, x, y, width, height);
        Layout_Next(&layout, 36, &x, &y, &width, &height); Button_SetBounds(&g_btnLoad,          x, y, width, height);
        Layout_Next(&layout, 36, &x, &y, &width, &height); Button_SetBounds(&g_btnDelete,        x, y, width, height);
    }

    static void EnsureLayout()
    {
        int x, y, w, h;
        Renderer_GetOverlayContentRect(&x, &y, &w, &h);
        if (!g_layoutCache.valid ||
            g_layoutCache.x != x || g_layoutCache.y != y ||
            g_layoutCache.width != w || g_layoutCache.height != h)
            RefreshLayout();
    }

    // Dados de teste: cada ciclo incrementa o score para mostrar que salvou novo valor.
    static int   g_cycleScore = 42;
    static float g_cycleSpeed = 3.14f;

    static void DoSave(int root, const char* subdir)
    {
        SaveLoad_SetBasePath(root, subdir);
        g_currentRoot = root;
        _snprintf_s(g_currentSubdir, sizeof(g_currentSubdir), _TRUNCATE, "%s", subdir);

        IXSaveWriter* w = SaveLoad_WriterCreate(TEST_VERSION);
        if (!w) { std::snprintf(g_statusLine, sizeof(g_statusLine), "Save failed: out of memory."); return; }

        SaveLoad_WriterPutInt  (w, "score", g_cycleScore);
        SaveLoad_WriterPutFloat(w, "speed", g_cycleSpeed);
        SaveLoad_WriterPutStr  (w, "name",  "DebugPlayer");

        int ok = SaveLoad_WriterSave(w, TEST_FILENAME);
        SaveLoad_WriterDestroy(w);

        if (ok)
        {
            std::snprintf(g_statusLine, sizeof(g_statusLine),
                "Saved! score=%d speed=%.2f -> %s",
                g_cycleScore, g_cycleSpeed, SaveLoad_GetBasePath());
            g_cycleScore += 10;   // proxima gravacao tera valor diferente
            g_cycleSpeed += 1.0f;
        }
        else
            std::snprintf(g_statusLine, sizeof(g_statusLine),
                "Save FAILED -> %s", SaveLoad_GetBasePath());

        std::snprintf(g_helpLine, sizeof(g_helpLine), "Path: %s | ESC menu", SaveLoad_GetBasePath());
    }

    static void DoLoad()
    {
        if (g_currentRoot < 0)
        {
            std::snprintf(g_statusLine, sizeof(g_statusLine), "Save first to choose a destination.");
            return;
        }

        // Reaplica caminho (pode ter mudado entre chamadas)
        SaveLoad_SetBasePath(g_currentRoot, g_currentSubdir);

        if (!SaveLoad_FileExists(TEST_FILENAME))
        {
            std::snprintf(g_statusLine, sizeof(g_statusLine), "File not found: %s", SaveLoad_GetBasePath());
            g_hasLoaded = 0;
            return;
        }

        IXSaveReader* r = SaveLoad_ReaderOpen(TEST_FILENAME);
        if (!r)
        {
            std::snprintf(g_statusLine, sizeof(g_statusLine), "Load FAILED (corrupt?).");
            g_hasLoaded = 0;
            return;
        }

        g_loadedScore = SaveLoad_ReaderGetInt  (r, "score");
        g_loadedSpeed = SaveLoad_ReaderGetFloat(r, "speed");
        SaveLoad_ReaderGetStr(r, "name", g_loadedName, sizeof(g_loadedName));
        unsigned int ver = SaveLoad_ReaderGetVersion(r);
        SaveLoad_ReaderDestroy(r);

        g_hasLoaded = 1;
        std::snprintf(g_statusLine, sizeof(g_statusLine),
            "Loaded v%u: score=%d speed=%.2f name=%s",
            ver, g_loadedScore, g_loadedSpeed, g_loadedName);
        std::snprintf(g_helpLine, sizeof(g_helpLine), "Path: %s | ESC menu", SaveLoad_GetBasePath());
    }

    static void DoDelete()
    {
        if (g_currentRoot < 0)
        {
            std::snprintf(g_statusLine, sizeof(g_statusLine), "Save first to choose a destination.");
            return;
        }
        SaveLoad_SetBasePath(g_currentRoot, g_currentSubdir);
        if (SaveLoad_FileDelete(TEST_FILENAME))
        {
            g_hasLoaded = 0;
            std::snprintf(g_statusLine, sizeof(g_statusLine), "File deleted from %s", SaveLoad_GetBasePath());
        }
        else
            std::snprintf(g_statusLine, sizeof(g_statusLine), "Delete failed (file missing?).");
    }
}

// ---------------------------------------------------------------------------
//  API publica
// ---------------------------------------------------------------------------

void DebugSaveLoadTest_Initialize()
{
    Button_Init(&g_btnSaveExe,       28, 140, 220, 36, "Save -> Exe folder");
    Button_Init(&g_btnSaveAppdata,   28, 186, 220, 36, "Save -> AppData");
    Button_Init(&g_btnSaveDocuments, 28, 232, 220, 36, "Save -> Documents");
    Button_Init(&g_btnLoad,          28, 278, 220, 36, "Load");
    Button_Init(&g_btnDelete,        28, 324, 220, 36, "Delete file");

    // Tons: verde-escuro para salvar, azul para load, vermelho para delete
    Button_SetColors(&g_btnSaveExe, 0xCC1E3A22, 0xCC285030, 0xCC346840);
    Button_SetColors(&g_btnSaveAppdata, 0xCC1E3030, 0xCC284060, 0xCC345580);
    Button_SetColors(&g_btnSaveDocuments, 0xCC1E2840, 0xCC284060, 0xCC2C5880);
    Button_SetColors(&g_btnLoad, 0xCC383020, 0xCC504228, 0xCC6A5832);
    Button_SetColors(&g_btnDelete, 0xCC4A1E1E, 0xCC682828, 0xCC883434);

    g_buttons[0] = &g_btnSaveExe;
    g_buttons[1] = &g_btnSaveAppdata;
    g_buttons[2] = &g_btnSaveDocuments;
    g_buttons[3] = &g_btnLoad;
    g_buttons[4] = &g_btnDelete;

    g_returnToMenu      = 0;
    g_hasLoaded         = 0;
    g_currentRoot       = -1;
    g_cycleScore        = 42;
    g_cycleSpeed        = 3.14f;
    g_layoutCache.valid = 0;

    std::snprintf(g_statusLine, sizeof(g_statusLine), "Save/Load test ready.");
    std::snprintf(g_helpLine,   sizeof(g_helpLine),   "Choose a destination to save, then Load or Delete | ESC menu");
    EnsureLayout();
}

void DebugSaveLoadTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased)
{
    EnsureLayout();

    if (Input_WasKeyPressed(VK_ESCAPE))
    {
        g_returnToMenu = 1;
        return;
    }

    if (Button_Handle(&g_btnSaveExe,       mouseX, mouseY, mouseDown, mouseReleased)) DoSave(IX_SAVE_PATH_EXE,       "");
    if (Button_Handle(&g_btnSaveAppdata,   mouseX, mouseY, mouseDown, mouseReleased)) DoSave(IX_SAVE_PATH_APPDATA,   TEST_APPNAME);
    if (Button_Handle(&g_btnSaveDocuments, mouseX, mouseY, mouseDown, mouseReleased)) DoSave(IX_SAVE_PATH_DOCUMENTS, TEST_APPNAME "\\saves");
    if (Button_Handle(&g_btnLoad,          mouseX, mouseY, mouseDown, mouseReleased)) DoLoad();
    if (Button_Handle(&g_btnDelete,        mouseX, mouseY, mouseDown, mouseReleased)) DoDelete();
}

void DebugSaveLoadTest_ApplyRendererState(float renderFPS, float frameMs)
{
    EnsureLayout();
    Renderer_SetOverlayText("InteractiveX r0.2", "Save/Load test", g_statusLine);
    Renderer_SetHelpText(g_helpLine);
    Renderer_SetDisplayOptions(1, 1, 1);
    Renderer_SetControlButton(NULL);
    Renderer_SetControlButtons(g_buttons, 5);
    Renderer_SetControlSliders(NULL, 0);
    Renderer_SetOverlayMetrics(renderFPS, frameMs, 0.0f);
}

int  DebugSaveLoadTest_ShouldReturnToMenu() { return g_returnToMenu; }
void DebugSaveLoadTest_ClearReturnToMenu()  { g_returnToMenu = 0;    }
