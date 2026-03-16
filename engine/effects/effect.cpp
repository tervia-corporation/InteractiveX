#include "effect.h"
#include "fire.h"

static FireSettings g_fireSettings;
static int g_fireSpeedMs = 33;
static int g_fireBlur = 1;

void Effect_InitializeFire(const FireSettings& settings)
{
    g_fireSettings = settings;
    Fire_Initialize(
        g_fireSettings.width,
        g_fireSettings.height,
        g_fireSettings.paletteSize,
        g_fireSettings.maxDecay,
        g_fireSettings.windStrength,
        g_fireSettings.baseIntensity
    );
    Fire_SetBlur(g_fireBlur);
}

void Effect_ResetFire()
{
    Effect_InitializeFire(g_fireSettings);
}

void Effect_UpdateFire()
{
    Fire_Update();
}

void Effect_SetFireSpeed(int milliseconds)
{
    if (milliseconds < 1)
        milliseconds = 1;

    g_fireSpeedMs = milliseconds;
}

int Effect_GetFireSpeed()
{
    return g_fireSpeedMs;
}

void Effect_SetFireWind(int windStrength)
{
    g_fireSettings.windStrength = windStrength;
    Fire_SetWind(windStrength);
}

int Effect_GetFireWind()
{
    return g_fireSettings.windStrength;
}

void Effect_SetFireBaseIntensity(int intensity)
{
    g_fireSettings.baseIntensity = intensity;
    Fire_SetBaseIntensity(intensity);
}

int Effect_GetFireBaseIntensity()
{
    return g_fireSettings.baseIntensity;
}

void Effect_SetFireDecay(int maxDecay)
{
    if (maxDecay < 1)
        maxDecay = 1;

    g_fireSettings.maxDecay = maxDecay;
    Fire_SetDecay(maxDecay);
}

int Effect_GetFireDecay()
{
    return g_fireSettings.maxDecay;
}

void Effect_SetFireBlur(int blurAmount)
{
    if (blurAmount < 0)
        blurAmount = 0;
    if (blurAmount > 3)
        blurAmount = 3;

    g_fireBlur = blurAmount;
    Fire_SetBlur(blurAmount);
}

int Effect_GetFireBlur()
{
    return g_fireBlur;
}


void Effect_SetFireSize(int width, int height)
{
    if (width < 16)
        width = 16;
    if (height < 16)
        height = 16;

    g_fireSettings.width = width;
    g_fireSettings.height = height;
    Effect_InitializeFire(g_fireSettings);
}

const FireSettings& Effect_GetFireSettings()
{
    return g_fireSettings;
}

const unsigned int* Effect_GetFireBuffer()
{
    return Fire_GetBuffer();
}

int Effect_GetFireWidth()
{
    return Fire_GetWidth();
}

int Effect_GetFireHeight()
{
    return Fire_GetHeight();
}
