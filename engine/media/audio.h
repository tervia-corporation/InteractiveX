#ifndef IX_MEDIA_AUDIO_H
#define IX_MEDIA_AUDIO_H

#include <windows.h>

// ---------------------------------------------------------------------------
//  Lifecycle
// ---------------------------------------------------------------------------
int  MediaAudio_Initialize(HWND hWnd);
void MediaAudio_Shutdown();

// ---------------------------------------------------------------------------
//  Music channel  (um arquivo por vez, loop, volume, pause/resume)
//
//  relativeAssetPath  caminho relativo ao assets root, ex: "music/theme.ogg"
//  loop               1 = loop infinito, 0 = toca uma vez
// ---------------------------------------------------------------------------
int  MediaAudio_MusicPlay(const char* relativeAssetPath, int loop);
void MediaAudio_MusicStop();
void MediaAudio_MusicPause();
void MediaAudio_MusicResume();
void MediaAudio_MusicSetVolume(float volume);   // 0.0 = mudo, 1.0 = normal
float MediaAudio_MusicGetVolume();
int  MediaAudio_MusicIsPlaying();
int  MediaAudio_MusicIsPaused();

// ---------------------------------------------------------------------------
//  SFX pool  (ate IX_SFX_VOICES vozes simultaneas, fire-and-forget)
//
//  relativeAssetPath  caminho relativo ao assets root, ex: "sfx/jump.wav"
//  volume             0.0 - 1.0, aplicado a esta voz especifica
//
//  Retorna 1 se disparou, 0 se falhou (arquivo nao encontrado, pool cheio
//  com todos os slots ainda tocando, ou audio nao inicializado).
// ---------------------------------------------------------------------------
#define IX_SFX_VOICES 8

int  MediaAudio_SFXPlay(const char* relativeAssetPath, float volume);
void MediaAudio_SFXSetMasterVolume(float volume);  // 0.0 - 1.0, afeta todas as vozes
float MediaAudio_SFXGetMasterVolume();

// ---------------------------------------------------------------------------
//  Status / diagnostico
// ---------------------------------------------------------------------------
const char* MediaAudio_GetLastError();
const char* MediaAudio_GetLastResolvedPath();

// ---------------------------------------------------------------------------
//  API legada  (mantida para compatibilidade com audio_test.cpp)
//  Internamente roteada para o canal de musica.
// ---------------------------------------------------------------------------
int  MediaAudio_PlayFile(const char* relativeAssetPath, int loop);
int  MediaAudio_PlayDemoWAV(int loop);
void MediaAudio_Stop();
void MediaAudio_Pause();
void MediaAudio_Resume();
int  MediaAudio_IsPaused();
int  MediaAudio_HasActiveFile();

#endif // IX_MEDIA_AUDIO_H
