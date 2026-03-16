#ifndef IX_CORE_ANIMATION_H
#define IX_CORE_ANIMATION_H

#ifdef __cplusplus
extern "C" {
#endif

struct IDirect3DTexture9;
struct IXEntity;

// ---------------------------------------------------------------------------
//  IXAnimation
//
//  Reproduz uma sequencia de frames de uma spritesheet.
//  Cada frame e um recorte retangular de tamanho fixo (frameW x frameH),
//  disposto em linhas e colunas na textura.
//
//  Layout da spritesheet:
//
//    col:  0      1      2      3
//    row 0 [fr 0][fr 1][fr 2][fr 3]
//    row 1 [fr 4][fr 5][fr 6][fr 7]
//
//  Uso tipico — animacao standalone:
//
//      IXAnimation anim;
//      Animation_Init(&anim, tex, 32, 32);   // frames de 32x32
//      Animation_AddClip(&anim, "idle",  0, 4, 8.0f,  1);  // frames 0-3, 8fps, loop
//      Animation_AddClip(&anim, "run",   4, 6, 12.0f, 1);  // frames 4-9, 12fps, loop
//      Animation_AddClip(&anim, "jump", 10, 3, 10.0f, 0);  // frames 10-12, once
//      Animation_Play(&anim, "run");
//
//      // a cada frame:
//      Animation_Update(&anim, Timer_GetDeltaSeconds());
//      Animation_Render(&anim, x, y, w, h, tint);
//
//  Uso com IXEntity — aplica o frame atual direto na entity:
//
//      Animation_ApplyToEntity(&anim, &player);
//      Entity_Render(&player);   // ou EntityList_RenderAll
//
//  Callbacks (opcionais):
//      Animation_SetOnClipEnd(&anim, MyCallback);  // chamado ao terminar once
// ---------------------------------------------------------------------------

#define IX_ANIM_MAX_CLIPS    16
#define IX_ANIM_NAME_LEN     32

typedef void (*IXAnimationCallback)(struct IXAnimation* anim, const char* clipName);

typedef struct IXAnimClip
{
    char  name[IX_ANIM_NAME_LEN];
    int   firstFrame;   // indice do primeiro frame na spritesheet
    int   frameCount;   // quantos frames tem o clip
    float fps;          // frames por segundo
    int   loop;         // 1 = loop, 0 = para no ultimo frame
} IXAnimClip;

typedef struct IXAnimation
{
    // --- Spritesheet ---
    struct IDirect3DTexture9* texture;
    int texW, texH;     // dimensoes reais da textura (preenchido em Init)
    int frameW, frameH; // tamanho de cada frame em pixels
    int cols;           // numero de colunas na spritesheet (calculado)

    // --- Clips registrados ---
    IXAnimClip clips[IX_ANIM_MAX_CLIPS];
    int        clipCount;

    // --- Estado de reproducao ---
    int   currentClip;   // indice em clips[], -1 = nenhum
    float timer;         // acumulador de tempo em segundos
    int   currentFrame;  // frame atual dentro do clip (0-based)
    int   finished;      // 1 se clip once chegou ao fim
    int   paused;

    // --- Callback ---
    IXAnimationCallback onClipEnd;

} IXAnimation;

// ---------------------------------------------------------------------------
//  Inicializacao
// ---------------------------------------------------------------------------
// Inicializa a animacao com a textura e tamanho de frame.
// A textura deve estar carregada antes de chamar Init.
// texW e texH sao consultados via GetLevelDesc — nao precisa passar.
void Animation_Init(IXAnimation* anim,
                    struct IDirect3DTexture9* texture,
                    int frameW, int frameH);

// Troca a textura (mantem clips e estado).
void Animation_SetTexture(IXAnimation* anim,
                           struct IDirect3DTexture9* texture);

// ---------------------------------------------------------------------------
//  Registro de clips
// ---------------------------------------------------------------------------
// Adiciona um clip. Retorna 1 em sucesso, 0 se o limite foi atingido.
// Se ja existir um clip com o mesmo nome, substitui.
int Animation_AddClip(IXAnimation* anim,
                      const char*  name,
                      int          firstFrame,
                      int          frameCount,
                      float        fps,
                      int          loop);

// ---------------------------------------------------------------------------
//  Controle de reproducao
// ---------------------------------------------------------------------------
// Inicia a reproducao do clip com o nome dado.
// Se ja estiver tocando o mesmo clip, reinicia.
// Retorna 1 em sucesso, 0 se o clip nao foi encontrado.
int  Animation_Play(IXAnimation* anim, const char* clipName);

// Reinicia o clip atual do frame 0.
void Animation_Restart(IXAnimation* anim);

void Animation_Pause(IXAnimation* anim);
void Animation_Resume(IXAnimation* anim);
void Animation_Stop(IXAnimation* anim);   // para e volta ao frame 0

// Define callback chamado quando um clip "once" termina.
void Animation_SetOnClipEnd(IXAnimation* anim, IXAnimationCallback cb);

// ---------------------------------------------------------------------------
//  Queries
// ---------------------------------------------------------------------------
const char* Animation_GetCurrentClipName(const IXAnimation* anim);
int         Animation_GetCurrentFrame(const IXAnimation* anim);
int         Animation_IsFinished(const IXAnimation* anim);  // true se clip once terminou
int         Animation_IsPaused(const IXAnimation* anim);

// ---------------------------------------------------------------------------
//  Logica  (chamar uma vez por frame)
// ---------------------------------------------------------------------------
void Animation_Update(IXAnimation* anim, float deltaSeconds);

// ---------------------------------------------------------------------------
//  Rendering
// ---------------------------------------------------------------------------
// Renderiza o frame atual no retangulo de destino (screen space ou world space
// dependendo se Camera_Attach foi chamado).
void Animation_Render(const IXAnimation* anim,
                      int dstX, int dstY, int dstW, int dstH,
                      unsigned int tint);

// Aplica srcX/Y/W/H do frame atual diretamente na IXEntity.
// Depois chame Entity_Render normalmente.
void Animation_ApplyToEntity(const IXAnimation* anim, struct IXEntity* entity);

#ifdef __cplusplus
}
#endif

#endif // IX_CORE_ANIMATION_H
