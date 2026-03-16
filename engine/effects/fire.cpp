#include "fire.h"
#include <cstdlib>
#include <ctime>
#include <cstring>

static int g_fireWidth = 0;
static int g_fireHeight = 0;
static int g_paletteSize = 37;
static int g_maxDecay = 4;
static int g_windStrength = 1;
static int g_baseIntensity = 36;
static int g_blurAmount = 1;

static int* g_fireIntensity = NULL;
static unsigned int* g_firePixels = NULL;
static unsigned int* g_fireTempPixels = NULL;
static unsigned int* g_firePalette = NULL;

static void Fire_CreatePalette()
{
    if (!g_firePalette)
        return;

    static unsigned int defaultPalette[37] =
    {
        0x00000000, 0x501F0707, 0x702F0F07, 0x90470F07, 0xA0571707,
        0xB0671F07, 0xC0771F07, 0xCC8F2707, 0xD89F2F07, 0xE0AF3F07,
        0xE8BF4707, 0xECC74707, 0xF0DF4F07, 0xF2DF5707, 0xF4DF5707,
        0xF6D75F07, 0xF8D7670F, 0xFACF6F0F, 0xFCCF770F, 0xFDCF7F0F,
        0xFDCF8717, 0xFEC78717, 0xFEC78F17, 0xFEC7971F, 0xFEBF9F1F,
        0xFEBF9F1F, 0xFEBFA727, 0xFEBFA727, 0xFEBFAF2F, 0xFEB7AF2F,
        0xFEB7B72F, 0xFEB7B737, 0xFECFCF6F, 0xFEDFDF9F, 0xFFEFEFC7,
        0xFFFFFFEF, 0xFFFFFFFF
    };

    int copyCount = g_paletteSize;
    if (copyCount > 37)
        copyCount = 37;

    for (int i = 0; i < copyCount; i++)
        g_firePalette[i] = defaultPalette[i];

    for (int i = 37; i < g_paletteSize; i++)
        g_firePalette[i] = 0xFFFFFFFF;
}

static void Fire_ResetBaseRow()
{
    if (!g_fireIntensity || g_fireWidth <= 0 || g_fireHeight <= 0)
        return;

    int baseValue = g_baseIntensity;
    if (baseValue < 0)
        baseValue = 0;
    if (baseValue >= g_paletteSize)
        baseValue = g_paletteSize - 1;

    int lastRow = (g_fireHeight - 1) * g_fireWidth;
    for (int x = 0; x < g_fireWidth; x++)
        g_fireIntensity[lastRow + x] = baseValue;
}

static unsigned int Fire_Mix4(unsigned int a, unsigned int b, unsigned int c, unsigned int d)
{
    unsigned int aa = ((a >> 24) & 0xFF) + ((b >> 24) & 0xFF) + ((c >> 24) & 0xFF) + ((d >> 24) & 0xFF);
    unsigned int rr = ((a >> 16) & 0xFF) + ((b >> 16) & 0xFF) + ((c >> 16) & 0xFF) + ((d >> 16) & 0xFF);
    unsigned int gg = ((a >> 8) & 0xFF) + ((b >> 8) & 0xFF) + ((c >> 8) & 0xFF) + ((d >> 8) & 0xFF);
    unsigned int bb = (a & 0xFF) + (b & 0xFF) + (c & 0xFF) + (d & 0xFF);

    return ((aa / 4) << 24) | ((rr / 4) << 16) | ((gg / 4) << 8) | (bb / 4);
}

static void Fire_ApplyBlurPass()
{
    if (!g_firePixels || !g_fireTempPixels)
        return;

    std::memcpy(g_fireTempPixels, g_firePixels, sizeof(unsigned int) * g_fireWidth * g_fireHeight);

    for (int y = 1; y < g_fireHeight - 1; y++)
    {
        for (int x = 1; x < g_fireWidth - 1; x++)
        {
            int idx = x + y * g_fireWidth;
            unsigned int center = g_firePixels[idx];
            unsigned int left = g_firePixels[idx - 1];
            unsigned int right = g_firePixels[idx + 1];
            unsigned int up = g_firePixels[idx - g_fireWidth];
            unsigned int down = g_firePixels[idx + g_fireWidth];
            unsigned int mixedA = Fire_Mix4(center, left, right, up);
            unsigned int mixedB = Fire_Mix4(center, left, right, down);
            g_fireTempPixels[idx] = Fire_Mix4(mixedA, mixedB, center, center);
        }
    }

    std::memcpy(g_firePixels, g_fireTempPixels, sizeof(unsigned int) * g_fireWidth * g_fireHeight);
}

void Fire_Initialize(int width, int height, int paletteSize, int maxDecay, int windStrength, int baseIntensity)
{
    std::srand((unsigned int)std::time(0));

    if (g_fireIntensity)  { delete[] g_fireIntensity;  g_fireIntensity = NULL; }
    if (g_firePixels)     { delete[] g_firePixels;     g_firePixels = NULL; }
    if (g_fireTempPixels) { delete[] g_fireTempPixels; g_fireTempPixels = NULL; }
    if (g_firePalette)    { delete[] g_firePalette;    g_firePalette = NULL; }

    g_fireWidth = width;
    g_fireHeight = height;
    g_paletteSize = (paletteSize < 2) ? 2 : paletteSize;
    g_maxDecay = (maxDecay < 1) ? 1 : maxDecay;
    g_windStrength = windStrength;
    g_baseIntensity = baseIntensity;

    g_fireIntensity = new int[g_fireWidth * g_fireHeight];
    g_firePixels = new unsigned int[g_fireWidth * g_fireHeight];
    g_fireTempPixels = new unsigned int[g_fireWidth * g_fireHeight];
    g_firePalette = new unsigned int[g_paletteSize];

    Fire_CreatePalette();

    for (int i = 0; i < g_fireWidth * g_fireHeight; i++)
    {
        g_fireIntensity[i] = 0;
        g_firePixels[i] = 0x00000000;
        g_fireTempPixels[i] = 0x00000000;
    }

    Fire_ResetBaseRow();
}

void Fire_SetWind(int windStrength)
{
    g_windStrength = windStrength;
}

void Fire_SetBaseIntensity(int intensity)
{
    g_baseIntensity = intensity;
    Fire_ResetBaseRow();
}

void Fire_SetDecay(int maxDecay)
{
    if (maxDecay < 1)
        maxDecay = 1;

    g_maxDecay = maxDecay;
}

void Fire_SetBlur(int blurAmount)
{
    if (blurAmount < 0)
        blurAmount = 0;
    if (blurAmount > 3)
        blurAmount = 3;

    g_blurAmount = blurAmount;
}

void Fire_Update()
{
    if (!g_fireIntensity || !g_firePixels)
        return;

    Fire_ResetBaseRow();

    for (int y = 1; y < g_fireHeight; y++)
    {
        for (int x = 0; x < g_fireWidth; x++)
        {
            int src = x + y * g_fireWidth;
            int decay = std::rand() % g_maxDecay;

            int lateral = 0;
            if (g_windStrength != 0)
            {
                int range = (g_windStrength < 0) ? -g_windStrength : g_windStrength;
                lateral = std::rand() % (range + 1);

                if (g_windStrength < 0)
                    lateral = -lateral;
            }
            else
            {
                lateral = (std::rand() % 3) - 1;
            }

            int dstX = x + lateral;
            int dstY = y - 1;

            if (dstX < 0) dstX = 0;
            if (dstX >= g_fireWidth) dstX = g_fireWidth - 1;

            int dst = dstX + dstY * g_fireWidth;

            int value = g_fireIntensity[src] - decay;
            if (value < 0)
                value = 0;

            g_fireIntensity[dst] = value;
        }
    }

    for (int i = 0; i < g_fireWidth * g_fireHeight; i++)
    {
        int intensity = g_fireIntensity[i];
        if (intensity < 0) intensity = 0;
        if (intensity >= g_paletteSize) intensity = g_paletteSize - 1;

        g_firePixels[i] = g_firePalette[intensity];
    }

    for (int i = 0; i < g_blurAmount; i++)
        Fire_ApplyBlurPass();
}

const unsigned int* Fire_GetBuffer()
{
    return g_firePixels;
}

int Fire_GetWidth()
{
    return g_fireWidth;
}

int Fire_GetHeight()
{
    return g_fireHeight;
}
