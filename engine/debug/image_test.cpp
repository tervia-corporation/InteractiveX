#include "image_test.h"
#include "../renderer.h"
#include "../core/input.h"
#include "../core/assets.h"
#include "../media/image.h"
#include "../gui.h"
#include <cstdio>
#include <cstring>

// ---------------------------------------------------------------------------
//  Estado
// ---------------------------------------------------------------------------
namespace
{
    static IXImage g_image;
    static int     g_returnToMenu = 0;
    static char    g_statusLine[256] = "Image test ready.";
    static char    g_helpLine[256]   = "Scroll zoom | Drag mover | R reset | ESC menu";

    // Viewport da imagem — posicao e zoom
    static float g_imgX    = 0.0f;   // offset de pan (pixels de tela)
    static float g_imgY    = 0.0f;
    static float g_zoom    = 1.0f;
    static float g_zoomTarget = 1.0f;

    // Drag
    static int   g_dragging    = 0;
    static int   g_dragStartX  = 0;
    static int   g_dragStartY  = 0;
    static float g_dragOriginX = 0.0f;
    static float g_dragOriginY = 0.0f;

    // Area de visualizacao (direita do overlay)
    static const float VIEW_MARGIN_LEFT = 380.0f;
    static const float VIEW_MARGIN      = 16.0f;

    static void ResetView()
    {
        if (!Image_IsLoaded(&g_image)) { g_imgX = 0; g_imgY = 0; g_zoom = 1.0f; g_zoomTarget = 1.0f; return; }

        int cw = 0, ch = 0;
        Renderer_GetClientSize(&cw, &ch);

        float availW = (float)cw - VIEW_MARGIN_LEFT - VIEW_MARGIN;
        float availH = (float)ch - VIEW_MARGIN * 2.0f;
        if (availW < 32.0f) availW = 32.0f;
        if (availH < 32.0f) availH = 32.0f;

        float iw = (float)g_image.width;
        float ih = (float)g_image.height;

        // Zoom para caber na area disponivel
        float zx = availW / iw;
        float zy = availH / ih;
        g_zoom      = (zx < zy ? zx : zy);
        g_zoomTarget = g_zoom;
        if (g_zoom > 1.0f) { g_zoom = 1.0f; g_zoomTarget = 1.0f; }

        // Centra
        g_imgX = VIEW_MARGIN_LEFT + (availW - iw * g_zoom) * 0.5f;
        g_imgY = VIEW_MARGIN      + (availH - ih * g_zoom) * 0.5f;
    }

    static void LoadImage()
    {
        char fullPath[MAX_PATH];
        Image_Release(&g_image);

        if (!Assets_BuildPath("demo.png", fullPath, MAX_PATH))
        {
            std::snprintf(g_statusLine, sizeof(g_statusLine), "Falha ao construir path de assets/demo.png");
            return;
        }
        if (!Assets_FileExists(fullPath))
        {
            std::snprintf(g_statusLine, sizeof(g_statusLine),
                "assets/demo.png nao encontrado — coloque o arquivo na pasta assets/");
            return;
        }
        if (!Image_LoadPNG(Renderer_GetDevice(), fullPath, &g_image))
        {
            std::snprintf(g_statusLine, sizeof(g_statusLine),
                "Falha ao carregar PNG: %s", Image_GetLastError());
            return;
        }
        std::snprintf(g_statusLine, sizeof(g_statusLine),
            "assets/demo.png | %dx%d px", g_image.width, g_image.height);
        ResetView();
    }
}

// ---------------------------------------------------------------------------
//  Public API
// ---------------------------------------------------------------------------
void DebugImageTest_Initialize()
{
    std::memset(&g_image, 0, sizeof(g_image));
    g_returnToMenu = 0;
    g_dragging     = 0;
    g_zoom         = 1.0f;
    g_zoomTarget   = 1.0f;
    Renderer_ClearDisplayImage();
    LoadImage();
}

void DebugImageTest_Update(int mouseX, int mouseY, int mouseDown, int mouseReleased)
{
    if (Input_WasKeyPressed(VK_ESCAPE)) { g_returnToMenu = 1; return; }

    // Reset
    if (Input_WasKeyPressed('R')) ResetView();

    // Zoom com scroll
    int wheel = Input_GetMouseWheelDelta();
    if (wheel != 0)
    {
        float factor = wheel > 0 ? 1.15f : (1.0f / 1.15f);
        float newZoom = g_zoomTarget * factor;
        if (newZoom < 0.05f) newZoom = 0.05f;
        if (newZoom > 32.0f) newZoom = 32.0f;

        // Zoom centrado no mouse
        if (Image_IsLoaded(&g_image))
        {
            float mx = (float)mouseX;
            float my = (float)mouseY;
            float ratio = newZoom / g_zoomTarget;
            g_imgX = mx - (mx - g_imgX) * ratio;
            g_imgY = my - (my - g_imgY) * ratio;
        }
        g_zoomTarget = newZoom;
    }

    // Zoom com teclado +/-
    if (Input_WasKeyPressed(VK_OEM_PLUS)  || Input_WasKeyPressed(VK_ADD))    g_zoomTarget *= 1.25f;
    if (Input_WasKeyPressed(VK_OEM_MINUS) || Input_WasKeyPressed(VK_SUBTRACT)) g_zoomTarget /= 1.25f;
    if (g_zoomTarget < 0.05f) g_zoomTarget = 0.05f;
    if (g_zoomTarget > 32.0f) g_zoomTarget = 32.0f;

    // Lerp do zoom (suave)
    g_zoom += (g_zoomTarget - g_zoom) * 0.25f;

    // Drag para pan
    if (mouseDown && !g_dragging)
    {
        g_dragging    = 1;
        g_dragStartX  = mouseX;
        g_dragStartY  = mouseY;
        g_dragOriginX = g_imgX;
        g_dragOriginY = g_imgY;
    }
    if (mouseReleased) g_dragging = 0;
    if (g_dragging)
    {
        g_imgX = g_dragOriginX + (float)(mouseX - g_dragStartX);
        g_imgY = g_dragOriginY + (float)(mouseY - g_dragStartY);
    }

    // Status
    if (Image_IsLoaded(&g_image))
    {
        // Coordenada de imagem sob o mouse
        float imgPixX = ((float)mouseX - g_imgX) / g_zoom;
        float imgPixY = ((float)mouseY - g_imgY) / g_zoom;
        std::snprintf(g_statusLine, sizeof(g_statusLine),
            "%dx%d px | Zoom: %.0f%% | Mouse na imagem: %.0f, %.0f",
            g_image.width, g_image.height,
            g_zoom * 100.0f,
            imgPixX, imgPixY);
    }
}

void DebugImageTest_Render()
{
    if (!Image_IsLoaded(&g_image)) return;

    int cw = 0, ch = 0;
    Renderer_GetClientSize(&cw, &ch);

    float dstX = g_imgX;
    float dstY = g_imgY;
    float dstW = (float)g_image.width  * g_zoom;
    float dstH = (float)g_image.height * g_zoom;

    // Fundo escuro na area de visualizacao
    float vx = VIEW_MARGIN_LEFT;
    float vy = VIEW_MARGIN;
    float vw = (float)cw - vx - VIEW_MARGIN;
    float vh = (float)ch - vy - VIEW_MARGIN;
    GUI_DrawFilledRect((int)vx, (int)vy, (int)vw, (int)vh, 0x22000000);
    GUI_DrawOutlinedRect((int)vx, (int)vy, (int)vw, (int)vh, 0x33FFFFFF);

    // Imagem
    Renderer_DrawSprite(g_image.texture,
        0, 0, 0, 0,
        (int)dstX, (int)dstY, (int)dstW, (int)dstH,
        0xFFFFFFFF);

    // Borda ao redor da imagem
    GUI_DrawOutlinedRect((int)dstX - 1, (int)dstY - 1,
        (int)dstW + 2, (int)dstH + 2, 0x55FFFFFF);

    // Grade de pixels quando muito ampliado (zoom >= 8x)
    if (g_zoom >= 8.0f)
    {
        float pixW = g_zoom;
        float pixH = g_zoom;
        // Linhas verticais
        for (float px = dstX; px <= dstX + dstW; px += pixW)
            GUI_DrawFilledRect((int)px, (int)dstY, 1, (int)dstH, 0x22FFFFFF);
        // Linhas horizontais
        for (float py = dstY; py <= dstY + dstH; py += pixH)
            GUI_DrawFilledRect((int)dstX, (int)py, (int)dstW, 1, 0x22FFFFFF);
    }
}

void DebugImageTest_ApplyRendererState(float renderFPS, float frameMs)
{
    Renderer_SetBackgroundColor(0xFF10161F);
    Renderer_SetOverlayText("InteractiveX r0.2", "Image test", g_statusLine);
    Renderer_SetHelpText(g_helpLine);
    Renderer_SetDisplayOptions(1, 1, 0);
    Renderer_SetControlButton(NULL);
    Renderer_SetControlButtons(NULL, 0);
    Renderer_SetControlSliders(NULL, 0);
    Renderer_SetOverlayMetrics(renderFPS, frameMs, 0.0f);
}

int  DebugImageTest_ShouldReturnToMenu() { return g_returnToMenu; }
void DebugImageTest_ClearReturnToMenu()
{
    Image_Release(&g_image);
    Renderer_ClearDisplayImage();
    g_returnToMenu = 0;
}
