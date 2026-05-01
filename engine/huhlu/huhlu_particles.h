#ifndef IX_HUHLU_PARTICLES_H
#define IX_HUHLU_PARTICLES_H
#ifdef __cplusplus
extern "C" {
#endif
void HuhluParticles_ResetExplosion(float x, float y, float z);
void HuhluParticles_Update(float dt);
void HuhluParticles_Render(float camYaw, float camPitch, float camDist);
#ifdef __cplusplus
}
#endif
#endif
