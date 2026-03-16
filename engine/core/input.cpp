#include "input.h"
#include <cstring>
#include <cstdio>

static int g_mouseX = 0;
static int g_mouseY = 0;
static int g_mouseWheelDelta = 0;

static unsigned char g_keyDown[256];
static unsigned char g_keyPressed[256];
static unsigned char g_keyReleased[256];

static unsigned char g_mouseDown[IX_MOUSE_BUTTON_COUNT];
static unsigned char g_mousePressed[IX_MOUSE_BUTTON_COUNT];
static unsigned char g_mouseReleased[IX_MOUSE_BUTTON_COUNT];

static int g_lastPressedKey = 0;
static int g_lastReleasedKey = 0;

static void Input_SetMouseButtonState(int button, int down)
{
    if (button < 0 || button >= IX_MOUSE_BUTTON_COUNT)
        return;

    if (down)
    {
        if (!g_mouseDown[button])
            g_mousePressed[button] = 1;
        g_mouseDown[button] = 1;
    }
    else
    {
        if (g_mouseDown[button])
            g_mouseReleased[button] = 1;
        g_mouseDown[button] = 0;
    }
}

static void Input_SetKeyState(int vk, int down)
{
    if (vk < 0 || vk > 255)
        return;

    if (down)
    {
        if (!g_keyDown[vk])
        {
            g_keyPressed[vk] = 1;
            g_lastPressedKey = vk;
        }
        g_keyDown[vk] = 1;
    }
    else
    {
        if (g_keyDown[vk])
        {
            g_keyReleased[vk] = 1;
            g_lastReleasedKey = vk;
        }
        g_keyDown[vk] = 0;
    }
}

void Input_Initialize()
{
    g_mouseX = 0;
    g_mouseY = 0;
    g_mouseWheelDelta = 0;
    g_lastPressedKey = 0;
    g_lastReleasedKey = 0;

    std::memset(g_keyDown, 0, sizeof(g_keyDown));
    std::memset(g_keyPressed, 0, sizeof(g_keyPressed));
    std::memset(g_keyReleased, 0, sizeof(g_keyReleased));
    std::memset(g_mouseDown, 0, sizeof(g_mouseDown));
    std::memset(g_mousePressed, 0, sizeof(g_mousePressed));
    std::memset(g_mouseReleased, 0, sizeof(g_mouseReleased));
}

void Input_BeginFrame()
{
    std::memset(g_keyPressed, 0, sizeof(g_keyPressed));
    std::memset(g_keyReleased, 0, sizeof(g_keyReleased));
    std::memset(g_mousePressed, 0, sizeof(g_mousePressed));
    std::memset(g_mouseReleased, 0, sizeof(g_mouseReleased));
    g_mouseWheelDelta = 0;
}

void Input_HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_MOUSEMOVE:
            g_mouseX = (int)(short)LOWORD(lParam);
            g_mouseY = (int)(short)HIWORD(lParam);
            break;

        case WM_LBUTTONDOWN:
            Input_SetMouseButtonState(IX_MOUSE_LEFT, 1);
            break;

        case WM_LBUTTONUP:
            Input_SetMouseButtonState(IX_MOUSE_LEFT, 0);
            break;

        case WM_RBUTTONDOWN:
            Input_SetMouseButtonState(IX_MOUSE_RIGHT, 1);
            break;

        case WM_RBUTTONUP:
            Input_SetMouseButtonState(IX_MOUSE_RIGHT, 0);
            break;

        case WM_MBUTTONDOWN:
            Input_SetMouseButtonState(IX_MOUSE_MIDDLE, 1);
            break;

        case WM_MBUTTONUP:
            Input_SetMouseButtonState(IX_MOUSE_MIDDLE, 0);
            break;

        case WM_MOUSEWHEEL:
            g_mouseWheelDelta += GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
            break;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            Input_SetKeyState((int)(wParam & 0xFF), 1);
            break;

        case WM_KEYUP:
        case WM_SYSKEYUP:
            Input_SetKeyState((int)(wParam & 0xFF), 0);
            break;

        case WM_KILLFOCUS:
        case WM_CAPTURECHANGED:
            std::memset(g_keyDown, 0, sizeof(g_keyDown));
            std::memset(g_mouseDown, 0, sizeof(g_mouseDown));
            break;
    }
}

int Input_GetMouseX() { return g_mouseX; }
int Input_GetMouseY() { return g_mouseY; }
int Input_GetMouseWheelDelta() { return g_mouseWheelDelta; }

int Input_IsMouseButtonDown(int button)
{
    if (button < 0 || button >= IX_MOUSE_BUTTON_COUNT)
        return 0;
    return g_mouseDown[button] != 0;
}

int Input_WasMouseButtonPressed(int button)
{
    if (button < 0 || button >= IX_MOUSE_BUTTON_COUNT)
        return 0;
    return g_mousePressed[button] != 0;
}

int Input_WasMouseButtonReleased(int button)
{
    if (button < 0 || button >= IX_MOUSE_BUTTON_COUNT)
        return 0;
    return g_mouseReleased[button] != 0;
}

int Input_IsKeyDown(int vk)
{
    if (vk < 0 || vk > 255)
        return 0;
    return g_keyDown[vk] != 0;
}

int Input_WasKeyPressed(int vk)
{
    if (vk < 0 || vk > 255)
        return 0;
    return g_keyPressed[vk] != 0;
}

int Input_WasKeyReleased(int vk)
{
    if (vk < 0 || vk > 255)
        return 0;
    return g_keyReleased[vk] != 0;
}

int Input_GetLastPressedKey() { return g_lastPressedKey; }
int Input_GetLastReleasedKey() { return g_lastReleasedKey; }

const char* Input_KeyToString(int vk)
{
    static char name[32];

    switch (vk)
    {
        case 0: return "None";
        case VK_SPACE: return "SPACE";
        case VK_ESCAPE: return "ESC";
        case VK_RETURN: return "ENTER";
        case VK_TAB: return "TAB";
        case VK_SHIFT: return "SHIFT";
        case VK_CONTROL: return "CTRL";
        case VK_MENU: return "ALT";
        case VK_LEFT: return "LEFT";
        case VK_RIGHT: return "RIGHT";
        case VK_UP: return "UP";
        case VK_DOWN: return "DOWN";
        case VK_LBUTTON: return "MOUSE1";
        case VK_RBUTTON: return "MOUSE2";
        case VK_MBUTTON: return "MOUSE3";
        default:
            break;
    }

    if (vk >= 'A' && vk <= 'Z')
    {
        name[0] = (char)vk;
        name[1] = '\0';
        return name;
    }

    if (vk >= '0' && vk <= '9')
    {
        name[0] = (char)vk;
        name[1] = '\0';
        return name;
    }

    std::snprintf(name, sizeof(name), "VK_%d", vk);
    return name;
}
