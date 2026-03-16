#include "audio.h"
#include "../core/assets.h"
#include "../third_party/miniaudio/miniaudio.h"
#include <cstdio>
#include <cstring>

// ---------------------------------------------------------------------------
//  Internal state
// ---------------------------------------------------------------------------
namespace
{
    // Engine miniaudio compartilhado entre musica e SFX
    static ma_engine g_engine;
    static int       g_initialized = 0;

    // Diagnostico
    static char g_lastError[256]        = "Audio offline";
    static char g_lastResolvedPath[512] = "";

    static void SetError(const char* text)
    {
        std::snprintf(g_lastError, sizeof(g_lastError), "%s", text ? text : "Unknown");
    }

    // -----------------------------------------------------------------------
    //  Utilitario: resolve caminho relativo -> caminho absoluto validado
    // -----------------------------------------------------------------------
    static int BuildFullPath(const char* relPath, char* outPath, int outSize)
    {
        if (!relPath || !relPath[0] || !outPath || outSize <= 0)
        {
            SetError("Invalid audio path");
            return 0;
        }
        if (!Assets_BuildPath(relPath, outPath, outSize))
        {
            SetError("Assets path build failed");
            return 0;
        }
        std::snprintf(g_lastResolvedPath, sizeof(g_lastResolvedPath), "%s", outPath);
        if (!Assets_FileExists(outPath))
        {
            std::snprintf(g_lastError, sizeof(g_lastError), "Missing: %s", relPath);
            return 0;
        }
        return 1;
    }

    // -----------------------------------------------------------------------
    //  Canal de musica
    // -----------------------------------------------------------------------
    static ma_sound g_music;
    static int      g_musicLoaded  = 0;
    static int      g_musicPaused  = 0;
    static float    g_musicVolume  = 1.0f;

    static void Music_Unload()
    {
        if (g_musicLoaded)
        {
            ma_sound_uninit(&g_music);
            g_musicLoaded = 0;
        }
        g_musicPaused = 0;
    }

    // -----------------------------------------------------------------------
    //  Pool de SFX  (round-robin, fire-and-forget)
    // -----------------------------------------------------------------------
    struct SFXVoice
    {
        ma_sound sound;
        int      loaded;
    };

    static SFXVoice g_sfxPool[IX_SFX_VOICES];
    static int      g_sfxNext        = 0;   // proximo slot a ser usado
    static float    g_sfxMasterVolume = 1.0f;

    // Retorna o indice do proximo slot disponivel (round-robin).
    // Se todos estiverem tocando, reutiliza o mais antigo (g_sfxNext).
    static int SFX_AcquireSlot()
    {
        // Prefere slot ja finalizado
        for (int i = 0; i < IX_SFX_VOICES; i++)
        {
            int idx = (g_sfxNext + i) % IX_SFX_VOICES;
            if (!g_sfxPool[idx].loaded)
                return idx;
            if (ma_sound_at_end(&g_sfxPool[idx].sound))
                return idx;
        }
        // Todos ocupados: rouba o mais antigo
        int stolen = g_sfxNext;
        g_sfxNext  = (g_sfxNext + 1) % IX_SFX_VOICES;
        return stolen;
    }

    static void SFX_ReleaseSlot(int idx)
    {
        if (idx < 0 || idx >= IX_SFX_VOICES) return;
        if (g_sfxPool[idx].loaded)
        {
            ma_sound_uninit(&g_sfxPool[idx].sound);
            g_sfxPool[idx].loaded = 0;
        }
    }

    static void SFX_ShutdownAll()
    {
        for (int i = 0; i < IX_SFX_VOICES; i++)
            SFX_ReleaseSlot(i);
    }

} // namespace

// ---------------------------------------------------------------------------
//  Lifecycle
// ---------------------------------------------------------------------------
int MediaAudio_Initialize(HWND hWnd)
{
    (void)hWnd;
    if (g_initialized) return 1;

    ma_result r = ma_engine_init(NULL, &g_engine);
    if (r != MA_SUCCESS)
    {
        SetError("miniaudio engine init failed");
        return 0;
    }

    g_initialized    = 1;
    g_musicLoaded    = 0;
    g_musicPaused    = 0;
    g_musicVolume    = 1.0f;
    g_sfxNext        = 0;
    g_sfxMasterVolume = 1.0f;

    std::memset(g_sfxPool, 0, sizeof(g_sfxPool));
    g_lastResolvedPath[0] = '\0';
    SetError("Audio ready");
    return 1;
}

void MediaAudio_Shutdown()
{
    if (!g_initialized) return;
    Music_Unload();
    SFX_ShutdownAll();
    ma_engine_uninit(&g_engine);
    g_initialized = 0;
    g_lastResolvedPath[0] = '\0';
    SetError("Audio offline");
}

// ---------------------------------------------------------------------------
//  Canal de musica
// ---------------------------------------------------------------------------
int MediaAudio_MusicPlay(const char* relativeAssetPath, int loop)
{
    if (!g_initialized) { SetError("Not initialized"); return 0; }

    char fullPath[512];
    if (!BuildFullPath(relativeAssetPath, fullPath, sizeof(fullPath))) return 0;

    Music_Unload();

    ma_result r = ma_sound_init_from_file(&g_engine, fullPath, 0, NULL, NULL, &g_music);
    if (r != MA_SUCCESS) { SetError("Music: failed to load"); return 0; }

    g_musicLoaded = 1;
    ma_sound_set_looping(&g_music, loop ? MA_TRUE : MA_FALSE);
    ma_sound_set_volume(&g_music, g_musicVolume);

    r = ma_sound_start(&g_music);
    if (r != MA_SUCCESS) { Music_Unload(); SetError("Music: failed to start"); return 0; }

    g_musicPaused = 0;
    std::snprintf(g_lastError, sizeof(g_lastError),
        "Music: %s | %s | ok", relativeAssetPath, loop ? "loop" : "once");
    return 1;
}

void MediaAudio_MusicStop()
{
    if (!g_initialized) return;
    Music_Unload();
    SetError("Music: stopped");
}

void MediaAudio_MusicPause()
{
    if (!g_initialized || !g_musicLoaded || g_musicPaused) return;
    if (ma_sound_stop(&g_music) == MA_SUCCESS)
    {
        g_musicPaused = 1;
        SetError("Music: paused");
    }
    else SetError("Music: pause failed");
}

void MediaAudio_MusicResume()
{
    if (!g_initialized || !g_musicLoaded || !g_musicPaused) return;
    if (ma_sound_start(&g_music) == MA_SUCCESS)
    {
        g_musicPaused = 0;
        SetError("Music: resumed");
    }
    else SetError("Music: resume failed");
}

void MediaAudio_MusicSetVolume(float volume)
{
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    g_musicVolume = volume;
    if (g_musicLoaded)
        ma_sound_set_volume(&g_music, g_musicVolume);
}

float MediaAudio_MusicGetVolume() { return g_musicVolume; }

int MediaAudio_MusicIsPlaying()
{
    if (!g_initialized || !g_musicLoaded || g_musicPaused) return 0;
    return ma_sound_is_playing(&g_music) ? 1 : 0;
}

int MediaAudio_MusicIsPaused() { return g_musicPaused; }

// ---------------------------------------------------------------------------
//  Pool de SFX
// ---------------------------------------------------------------------------
int MediaAudio_SFXPlay(const char* relativeAssetPath, float volume)
{
    if (!g_initialized) { SetError("Not initialized"); return 0; }

    char fullPath[512];
    if (!BuildFullPath(relativeAssetPath, fullPath, sizeof(fullPath))) return 0;

    int idx = SFX_AcquireSlot();
    SFX_ReleaseSlot(idx);   // libera o slot se estava em uso

    ma_result r = ma_sound_init_from_file(&g_engine, fullPath, 0, NULL, NULL, &g_sfxPool[idx].sound);
    if (r != MA_SUCCESS) { SetError("SFX: failed to load"); return 0; }

    g_sfxPool[idx].loaded = 1;

    float finalVol = volume * g_sfxMasterVolume;
    if (finalVol < 0.0f) finalVol = 0.0f;
    if (finalVol > 1.0f) finalVol = 1.0f;
    ma_sound_set_volume(&g_sfxPool[idx].sound, finalVol);

    r = ma_sound_start(&g_sfxPool[idx].sound);
    if (r != MA_SUCCESS)
    {
        SFX_ReleaseSlot(idx);
        SetError("SFX: failed to start");
        return 0;
    }

    // Avanca o cursor round-robin para o proximo slot
    g_sfxNext = (idx + 1) % IX_SFX_VOICES;
    return 1;
}

void MediaAudio_SFXSetMasterVolume(float volume)
{
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    g_sfxMasterVolume = volume;
}

float MediaAudio_SFXGetMasterVolume() { return g_sfxMasterVolume; }

// ---------------------------------------------------------------------------
//  Status / diagnostico
// ---------------------------------------------------------------------------
const char* MediaAudio_GetLastError()        { return g_lastError;        }
const char* MediaAudio_GetLastResolvedPath() { return g_lastResolvedPath; }

// ---------------------------------------------------------------------------
//  API legada  (roteada para o canal de musica)
// ---------------------------------------------------------------------------
int MediaAudio_PlayFile(const char* relativeAssetPath, int loop)
{
    return MediaAudio_MusicPlay(relativeAssetPath, loop);
}

int MediaAudio_PlayDemoWAV(int loop)
{
    return MediaAudio_MusicPlay("demo.wav", loop);
}

void MediaAudio_Stop()    { MediaAudio_MusicStop();   }
void MediaAudio_Pause()   { MediaAudio_MusicPause();  }
void MediaAudio_Resume()  { MediaAudio_MusicResume(); }
int  MediaAudio_IsPaused(){ return MediaAudio_MusicIsPaused(); }

int MediaAudio_HasActiveFile()
{
    if (!g_initialized || !g_musicLoaded) return 0;
    if (g_musicPaused) return 1;
    // Verifica se ainda esta tocando (som nao-loop pode ter terminado)
    if (!ma_sound_is_looping(&g_music) && ma_sound_at_end(&g_music))
    {
        Music_Unload();
        SetError("Music: playback finished");
        return 0;
    }
    return 1;
}
