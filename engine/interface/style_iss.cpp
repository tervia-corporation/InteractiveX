#include "style_iss.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

#define IX_ISS_MAX_RULES 512

static IXISSRule g_rules[IX_ISS_MAX_RULES];
static int g_ruleCount = 0;
static int g_viewW = 1280;
static int g_viewH = 720;
static char g_lastError[256] = "ISS: ready";

static void SetError(const char* e){ std::snprintf(g_lastError, sizeof(g_lastError), "%s", e ? e : "ISS: error"); }

void ISS_Clear(){ g_ruleCount = 0; SetError("ISS: cleared"); }
int ISS_GetRuleCount(){ return g_ruleCount; }
const IXISSRule* ISS_GetRule(int index){ return (index >= 0 && index < g_ruleCount) ? &g_rules[index] : 0; }
const char* ISS_GetLastError(){ return g_lastError; }
void ISS_SetViewport(int width, int height){ if (width > 0) g_viewW = width; if (height > 0) g_viewH = height; }

static const char* FindValue(const char* selector, const char* property)
{
    for (int i = g_ruleCount - 1; i >= 0; --i)
    {
        if (std::strcmp(g_rules[i].selector, selector) == 0 && std::strcmp(g_rules[i].property, property) == 0)
            return g_rules[i].value;
    }
    return 0;
}


static void TrimLeft(char* s)
{
    while (s && *s == ' ' || (s && *s == '	')) std::memmove(s, s + 1, std::strlen(s));
}

int ISS_GetString(const char* selector, const char* property, char* outValue, int outValueSize)
{
    if (!selector || !property || !outValue || outValueSize <= 0) return 0;
    const char* v = FindValue(selector, property);
    if (!v) return 0;
    std::snprintf(outValue, outValueSize, "%s", v);
    return 1;
}

static int HexNibble(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

int ISS_ParseColor(const char* value, unsigned int* outARGB)
{
    if (!value || !outARGB) return 0;
    if (value[0] == '#')
    {
        int len = (int)std::strlen(value);
        if (len == 7) // #RRGGBB
        {
            int rr = HexNibble(value[1]) * 16 + HexNibble(value[2]);
            int gg = HexNibble(value[3]) * 16 + HexNibble(value[4]);
            int bb = HexNibble(value[5]) * 16 + HexNibble(value[6]);
            if (rr < 0 || gg < 0 || bb < 0) return 0;
            *outARGB = 0xFF000000u | ((unsigned int)rr << 16) | ((unsigned int)gg << 8) | (unsigned int)bb;
            return 1;
        }
        if (len == 9) // #AARRGGBB
        {
            unsigned int v = 0;
            for (int i = 1; i < 9; ++i)
            {
                int n = HexNibble(value[i]);
                if (n < 0) return 0;
                v = (v << 4) | (unsigned int)n;
            }
            *outARGB = v;
            return 1;
        }
    }
    if (std::strcmp(value, "white") == 0) { *outARGB = 0xFFFFFFFFu; return 1; }
    if (std::strcmp(value, "black") == 0) { *outARGB = 0xFF000000u; return 1; }
    if (std::strcmp(value, "transparent") == 0) { *outARGB = 0x00000000u; return 1; }
    return 0;
}

int ISS_GetColor(const char* selector, const char* property, unsigned int* outARGB)
{
    char tmp[128];
    if (!ISS_GetString(selector, property, tmp, sizeof(tmp))) return 0;
    return ISS_ParseColor(tmp, outARGB);
}

int ISS_GetBackgroundImage(const char* selector, char* outPath, int outPathSize)
{
    char tmp[128];
    if (!ISS_GetString(selector, "background-image", tmp, sizeof(tmp))) return 0;

    const char* p = tmp;
    if (std::strncmp(p, "url(", 4) == 0)
    {
        p += 4;
        const char* end = std::strrchr(p, ')');
        if (!end) return 0;
        int n = (int)(end - p);
        if (n <= 0 || n >= outPathSize) return 0;
        std::memcpy(outPath, p, (size_t)n);
        outPath[n] = 0;
        if (outPath[0] == '"' || outPath[0] == ''')
        {
            std::memmove(outPath, outPath + 1, std::strlen(outPath));
            size_t l = std::strlen(outPath);
            if (l && (outPath[l-1] == '"' || outPath[l-1] == ''')) outPath[l-1] = 0;
        }
        return 1;
    }

    std::snprintf(outPath, outPathSize, "%s", tmp);
    return 1;
}

int ISS_ResolveLength(const char* value, int axisSize, int fontBase)
{
    if (!value || !value[0]) return 0;
    char* end = 0;
    double n = std::strtod(value, &end);
    if (end == value) return 0;

    if (std::strncmp(end, "px", 2) == 0 || *end == 0) return (int)(n + 0.5);
    if (std::strncmp(end, "%", 1) == 0) return (int)((axisSize * n / 100.0) + 0.5);
    if (std::strncmp(end, "vw", 2) == 0) return (int)((g_viewW * n / 100.0) + 0.5);
    if (std::strncmp(end, "vh", 2) == 0) return (int)((g_viewH * n / 100.0) + 0.5);
    if (std::strncmp(end, "rem", 3) == 0) return (int)((fontBase * n) + 0.5);
    return (int)(n + 0.5);
}

int ISS_ComputeRect(const char* selector, IXISSRect* outRect)
{
    if (!selector || !outRect) return 0;
    const char* widthV = FindValue(selector, "width");
    const char* heightV = FindValue(selector, "height");
    const char* leftV = FindValue(selector, "left");
    const char* topV = FindValue(selector, "top");
    const char* rightV = FindValue(selector, "right");
    const char* bottomV = FindValue(selector, "bottom");
    const char* posV = FindValue(selector, "position");

    int w = widthV ? ISS_ResolveLength(widthV, g_viewW, 16) : 0;
    int h = heightV ? ISS_ResolveLength(heightV, g_viewH, 16) : 0;
    int x = leftV ? ISS_ResolveLength(leftV, g_viewW, 16) : 0;
    int y = topV ? ISS_ResolveLength(topV, g_viewH, 16) : 0;

    if (rightV && !leftV) x = g_viewW - ISS_ResolveLength(rightV, g_viewW, 16) - w;
    if (bottomV && !topV) y = g_viewH - ISS_ResolveLength(bottomV, g_viewH, 16) - h;

    if (posV && std::strcmp(posV, "center") == 0)
    {
        x = (g_viewW - w) / 2;
        y = (g_viewH - h) / 2;
    }

    outRect->x = x; outRect->y = y; outRect->width = w; outRect->height = h;
    return 1;
}


static int ParseAlign(const char* v)
{
    if (!v) return 0;
    if (std::strcmp(v, "center") == 0) return 1;
    if (std::strcmp(v, "end") == 0 || std::strcmp(v, "right") == 0 || std::strcmp(v, "bottom") == 0) return 2;
    return 0;
}

int ISS_ComputeBoxStyle(const char* selector, IXISSBoxStyle* outStyle)
{
    if (!selector || !outStyle) return 0;
    std::memset(outStyle, 0, sizeof(*outStyle));

    char buf[128];
    outStyle->width = ISS_GetString(selector, "width", buf, sizeof(buf)) ? ISS_ResolveLength(buf, g_viewW, 16) : 0;
    outStyle->height = ISS_GetString(selector, "height", buf, sizeof(buf)) ? ISS_ResolveLength(buf, g_viewH, 16) : 0;

    if (ISS_GetString(selector, "weight", buf, sizeof(buf))) outStyle->weight = std::atoi(buf);
    if (ISS_GetString(selector, "align-x", buf, sizeof(buf))) outStyle->alignX = ParseAlign(buf);
    if (ISS_GetString(selector, "align-y", buf, sizeof(buf))) outStyle->alignY = ParseAlign(buf);
    if (ISS_GetString(selector, "outline-width", buf, sizeof(buf))) outStyle->borderWidth = std::atoi(buf);
    if (!ISS_GetColor(selector, "outline-color", &outStyle->borderColor)) outStyle->borderColor = 0x00000000u;

    unsigned int gt = 0, gb = 0;
    if (ISS_GetColor(selector, "gradient-top", &gt) && ISS_GetColor(selector, "gradient-bottom", &gb))
    {
        outStyle->hasGradient = 1;
        outStyle->gradientTop = gt;
        outStyle->gradientBottom = gb;
    }

    return 1;
}

int ISS_LayoutStackVertical(const char** selectors, int count, IXISSRect container, int gap, IXISSRect* outRects)
{
    if (!selectors || count <= 0 || !outRects) return 0;

    int totalFixed = 0;
    int totalWeight = 0;
    for (int i = 0; i < count; ++i)
    {
        IXISSBoxStyle st;
        ISS_ComputeBoxStyle(selectors[i], &st);
        if (st.weight > 0) totalWeight += st.weight;
        else totalFixed += st.height;
    }

    int totalGap = gap > 0 ? gap * (count - 1) : 0;
    int remaining = container.height - totalFixed - totalGap;
    if (remaining < 0) remaining = 0;

    int y = container.y;
    for (int i = 0; i < count; ++i)
    {
        IXISSBoxStyle st;
        ISS_ComputeBoxStyle(selectors[i], &st);
        int h = st.height;
        if (st.weight > 0 && totalWeight > 0) h = (remaining * st.weight) / totalWeight;
        int w = st.width > 0 ? st.width : container.width;

        int x = container.x;
        if (st.alignX == 1) x = container.x + (container.width - w) / 2;
        else if (st.alignX == 2) x = container.x + (container.width - w);

        outRects[i].x = x;
        outRects[i].y = y;
        outRects[i].width = w;
        outRects[i].height = h;

        y += h + gap;
    }
    return 1;
}

int ISS_LoadFile(const char* path)
{
    FILE* f = std::fopen(path, "rb");
    if (!f) { SetError("ISS: cannot open file"); return 0; }

    ISS_Clear();
    char line[512];
    char selector[64] = "";

    while (std::fgets(line, sizeof(line), f))
    {
        char* p = line;
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
        if (!*p || *p == '#' || (*p == '/' && *(p+1) == '/')) continue;

        char* brace = std::strchr(p, '{');
        if (brace)
        {
            *brace = 0;
            std::snprintf(selector, sizeof(selector), "%s", p);
            for (int i=(int)std::strlen(selector)-1; i>=0 && (selector[i]==' '||selector[i]=='\t'); --i) selector[i]=0;
            continue;
        }
        if (std::strchr(p, '}')) { selector[0] = 0; continue; }

        char* colon = std::strchr(p, ':');
        char* semi = std::strchr(p, ';');
        if (!colon || !semi || semi < colon || !selector[0]) continue;
        if (g_ruleCount >= IX_ISS_MAX_RULES) { SetError("ISS: rule limit reached"); break; }

        *colon = 0; *semi = 0;
        IXISSRule* r = &g_rules[g_ruleCount++];
        std::snprintf(r->selector, sizeof(r->selector), "%s", selector);
        std::snprintf(r->property, sizeof(r->property), "%s", p);
        std::snprintf(r->value, sizeof(r->value), "%s", colon + 1);

        TrimLeft(r->property);
        TrimLeft(r->value);
    }

    std::fclose(f);
    SetError("ISS: ok");
    return 1;
}
