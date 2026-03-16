#ifndef IX_FIRE_H
#define IX_FIRE_H

void Fire_Initialize(int width, int height, int paletteSize, int maxDecay, int windStrength, int baseIntensity);
void Fire_Update();

void Fire_SetWind(int windStrength);
void Fire_SetBaseIntensity(int intensity);
void Fire_SetDecay(int maxDecay);
void Fire_SetBlur(int blurAmount);

const unsigned int* Fire_GetBuffer();
int Fire_GetWidth();
int Fire_GetHeight();

#endif
