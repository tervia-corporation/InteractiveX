#ifndef IX_EFFECT_H
#define IX_EFFECT_H

struct FireSettings
{
    int width;
    int height;
    int paletteSize;
    int maxDecay;
    int windStrength;
    int baseIntensity;
};

void Effect_InitializeFire(const FireSettings& settings);
void Effect_ResetFire();
void Effect_UpdateFire();

void Effect_SetFireSpeed(int milliseconds);
int  Effect_GetFireSpeed();

void Effect_SetFireWind(int windStrength);
int  Effect_GetFireWind();
void Effect_SetFireBaseIntensity(int intensity);
int  Effect_GetFireBaseIntensity();
void Effect_SetFireDecay(int maxDecay);
int  Effect_GetFireDecay();
void Effect_SetFireBlur(int blurAmount);
int  Effect_GetFireBlur();
void Effect_SetFireSize(int width, int height);
const FireSettings& Effect_GetFireSettings();

const unsigned int* Effect_GetFireBuffer();
int Effect_GetFireWidth();
int Effect_GetFireHeight();

#endif
