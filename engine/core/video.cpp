#include "video.h"
#include "../renderer.h"
#include <cstring>

// ---------------------------------------------------------------------------
//  Estado interno
// ---------------------------------------------------------------------------
static HWND g_hWnd       = NULL;
static int  g_fullscreen = 0;
static int  g_vsync      = 0;
static int  g_lockResize = 0;
static int  g_resW       = 0;
static int  g_resH       = 0;
static int  g_minW       = 640;
static int  g_minH       = 480;

// Estilo original da janela (salvo antes de fullscreen)
static LONG     g_savedStyle   = 0;
static LONG     g_savedExStyle = 0;
static RECT     g_savedRect    = {0,0,0,0};
static int      g_wasMaximized = 0;

// ---------------------------------------------------------------------------
//  Helpers internos
// ---------------------------------------------------------------------------
static void GetMonitorRect(HWND hWnd, RECT* outRect)
{
    HMONITOR mon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(mon, &mi);
    *outRect = mi.rcMonitor;
}

static void ApplyWindowedMode()
{
    // Restaura estilo salvo
    SetWindowLong(g_hWnd, GWL_STYLE,   g_savedStyle);
    SetWindowLong(g_hWnd, GWL_EXSTYLE, g_savedExStyle);

    if (g_wasMaximized)
    {
        ShowWindow(g_hWnd, SW_MAXIMIZE);
    }
    else
    {
        // Aplica resolucao desejada
        int w = g_resW, h = g_resH;
        if (w <= 0 || h <= 0)
        {
            w = g_savedRect.right  - g_savedRect.left;
            h = g_savedRect.bottom - g_savedRect.top;
        }

        // Calcula tamanho da janela incluindo decoracoes
        RECT rc = {0, 0, w, h};
        AdjustWindowRectEx(&rc,
            (DWORD)GetWindowLong(g_hWnd, GWL_STYLE),
            FALSE,
            (DWORD)GetWindowLong(g_hWnd, GWL_EXSTYLE));

        int winW = rc.right  - rc.left;
        int winH = rc.bottom - rc.top;

        // Centraliza na tela
        RECT mon;
        GetMonitorRect(g_hWnd, &mon);
        int cx = mon.left + ((mon.right  - mon.left) - winW) / 2;
        int cy = mon.top  + ((mon.bottom - mon.top)  - winH) / 2;

        SetWindowPos(g_hWnd, HWND_NOTOPMOST,
            cx, cy, winW, winH,
            SWP_SHOWWINDOW | SWP_FRAMECHANGED);
    }
}

static void ApplyFullscreenMode()
{
    // Salva estado atual
    g_savedStyle   = GetWindowLong(g_hWnd, GWL_STYLE);
    g_savedExStyle = GetWindowLong(g_hWnd, GWL_EXSTYLE);
    g_wasMaximized = IsZoomed(g_hWnd);
    GetWindowRect(g_hWnd, &g_savedRect);

    // Remove todas as decoracoes
    SetWindowLong(g_hWnd, GWL_STYLE,
        g_savedStyle & ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE |
                         WS_MAXIMIZE | WS_SYSMENU));
    SetWindowLong(g_hWnd, GWL_EXSTYLE,
        g_savedExStyle & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE |
                           WS_EX_CLIENTEDGE    | WS_EX_STATICEDGE));

    // Expande para o monitor inteiro
    RECT mon;
    GetMonitorRect(g_hWnd, &mon);
    SetWindowPos(g_hWnd, HWND_TOP,
        mon.left, mon.top,
        mon.right  - mon.left,
        mon.bottom - mon.top,
        SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOOWNERZORDER);
}

// ---------------------------------------------------------------------------
//  API publica
// ---------------------------------------------------------------------------
void Video_Initialize(HWND hWnd)
{
    g_hWnd = hWnd;

    // Captura tamanho atual como resolucao inicial
    RECT rc;
    GetClientRect(hWnd, &rc);
    g_resW = rc.right  - rc.left;
    g_resH = rc.bottom - rc.top;

    // Salva estilo inicial para restauracao
    g_savedStyle   = GetWindowLong(hWnd, GWL_STYLE);
    g_savedExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
    GetWindowRect(hWnd, &g_savedRect);
}

void Video_SetFullscreen(int fs)    { g_fullscreen = fs ? 1 : 0; }
void Video_SetVSync(int vs)         { g_vsync      = vs ? 1 : 0; }
void Video_LockResize(int locked)   { g_lockResize = locked ? 1 : 0; }
void Video_SetMinSize(int mw, int mh)
{
    g_minW = (mw > 0) ? mw : 1;
    g_minH = (mh > 0) ? mh : 1;
}
void Video_SetResolution(int w, int h)
{
    if (w > 0) g_resW = w;
    if (h > 0) g_resH = h;
}

int  Video_IsFullscreen()            { return g_fullscreen; }
int  Video_IsVSync()                 { return g_vsync; }
int  Video_IsResizeLocked()          { return g_lockResize; }
void Video_GetResolution(int* w, int* h)
{
    if (w) *w = g_resW;
    if (h) *h = g_resH;
}

int Video_Apply()
{
    if (!g_hWnd) return 0;

    // 1. Aplica modo de janela
    if (g_fullscreen)
        ApplyFullscreenMode();
    else
        ApplyWindowedMode();

    // 2. Sincroniza renderer com o novo tamanho do client rect
    RECT rc;
    GetClientRect(g_hWnd, &rc);
    int w = rc.right  - rc.left;
    int h = rc.bottom - rc.top;
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;

    // 3. Aplica vsync e faz reset do device D3D
    Renderer_SetVSync(g_vsync);
    Renderer_HandleResize(w, h);

    return 1;
}

int Video_GetAvailableModes(IXVideoMode* outModes, int maxCount)
{
    if (!outModes || maxCount <= 0) return 0;

    int count = 0;
    DEVMODEA dm;
    ZeroMemory(&dm, sizeof(dm));
    dm.dmSize = sizeof(dm);

    for (DWORD i = 0; EnumDisplaySettingsA(NULL, i, &dm); i++)
    {
        if (dm.dmBitsPerPel != 32) continue;
        if ((int)dm.dmPelsWidth  < 640) continue;
        if ((int)dm.dmPelsHeight < 480) continue;

        // Filtra duplicatas (mesma resolucao, refresh diferente: mantém maior)
        int found = 0;
        for (int j = 0; j < count; j++)
        {
            if (outModes[j].width == (int)dm.dmPelsWidth &&
                outModes[j].height == (int)dm.dmPelsHeight)
            {
                if ((int)dm.dmDisplayFrequency > outModes[j].refreshRate)
                    outModes[j].refreshRate = (int)dm.dmDisplayFrequency;
                found = 1;
                break;
            }
        }

        if (!found && count < maxCount)
        {
            outModes[count].width       = (int)dm.dmPelsWidth;
            outModes[count].height      = (int)dm.dmPelsHeight;
            outModes[count].refreshRate = (int)dm.dmDisplayFrequency;
            count++;
        }
    }
    return count;
}

void Video_HandleGetMinMaxInfo(LPARAM lParam)
{
    MINMAXINFO* mmi = (MINMAXINFO*)lParam;

    if (g_lockResize && !g_fullscreen)
    {
        // Bloqueia: tamanho minimo e maximo sao iguais
        RECT rc = {0, 0, g_resW, g_resH};
        AdjustWindowRect(&rc, (DWORD)GetWindowLong(g_hWnd, GWL_STYLE), FALSE);
        int winW = rc.right  - rc.left;
        int winH = rc.bottom - rc.top;
        mmi->ptMinTrackSize.x = winW;
        mmi->ptMinTrackSize.y = winH;
        mmi->ptMaxTrackSize.x = winW;
        mmi->ptMaxTrackSize.y = winH;
    }
    else
    {
        // Apenas impoe minimo
        RECT rc = {0, 0, g_minW, g_minH};
        AdjustWindowRect(&rc, (DWORD)GetWindowLong(g_hWnd, GWL_STYLE), FALSE);
        mmi->ptMinTrackSize.x = rc.right  - rc.left;
        mmi->ptMinTrackSize.y = rc.bottom - rc.top;
    }
}

void Video_HandleSizing(WPARAM wParam, LPARAM lParam)
{
    // Se resize bloqueado, força o rect de volta ao tamanho fixo
    if (!g_lockResize || g_fullscreen) return;

    RECT* r = (RECT*)lParam;
    RECT rc = {0, 0, g_resW, g_resH};
    AdjustWindowRect(&rc, (DWORD)GetWindowLong(g_hWnd, GWL_STYLE), FALSE);
    int winW = rc.right  - rc.left;
    int winH = rc.bottom - rc.top;

    switch (wParam)
    {
        case WMSZ_RIGHT:
        case WMSZ_BOTTOMRIGHT:
        case WMSZ_BOTTOM:
            r->right  = r->left + winW;
            r->bottom = r->top  + winH;
            break;
        case WMSZ_LEFT:
        case WMSZ_TOPLEFT:
        case WMSZ_TOP:
            r->left = r->right  - winW;
            r->top  = r->bottom - winH;
            break;
        case WMSZ_TOPRIGHT:
            r->right = r->left  + winW;
            r->top   = r->bottom - winH;
            break;
        case WMSZ_BOTTOMLEFT:
            r->left   = r->right  - winW;
            r->bottom = r->top    + winH;
            break;
    }
}
