#ifndef IX_CORE_PARTICLES_H
#define IX_CORE_PARTICLES_H

#ifdef __cplusplus
extern "C" {
#endif

struct IDirect3DTexture9;

// ---------------------------------------------------------------------------
//  IXParticle  (interno — nao usar diretamente)
// ---------------------------------------------------------------------------
typedef struct IXParticle
{
    float x, y;         // posicao
    float vx, vy;       // velocidade px/s
    float ax, ay;       // aceleracao px/s^2 (gravidade, vento)
    float life;         // vida restante em segundos
    float lifeMax;      // vida total (para calcular t = 1 - life/lifeMax)
    int   size;         // tamanho atual em pixels
    int   sizeEnd;      // tamanho no fim da vida
    unsigned int colorStart; // ARGB no nascimento
    unsigned int colorEnd;   // ARGB na morte
    int   active;
} IXParticle;

// ---------------------------------------------------------------------------
//  IXEmitter
//
//  Emissor de particulas com buffer de tamanho fixo (sem malloc).
//  O buffer e fornecido pelo usuario — normalmente um array estatico.
//
//  Uso tipico:
//
//      #define MAX_PARTICLES 300
//      static IXParticle buf[MAX_PARTICLES];
//      static IXEmitter  emitter;
//
//      Emitter_Init(&emitter, buf, MAX_PARTICLES);
//      Emitter_SetPosition(&emitter, 400.0f, 300.0f);
//      Emitter_SetSpawnRate(&emitter, 60.0f);
//      Emitter_SetLifetime(&emitter, 0.4f, 1.0f);
//      Emitter_SetVelocity(&emitter, -40.0f, 40.0f, -120.0f, -60.0f);
//      Emitter_SetSize(&emitter, 3, 8, 1);
//      Emitter_SetColorOverLife(&emitter, 0xFFFF8800, 0x00FF2200);
//      Emitter_SetGravity(&emitter, 0.0f, 200.0f);
//
//      // a cada frame:
//      Emitter_Update(&emitter, Timer_GetDeltaSeconds());
//      Emitter_Render(&emitter);
//
//  Burst (explosao unica):
//      Emitter_Burst(&emitter, 80);   // dispara 80 particulas de uma vez
//      Emitter_SetSpawnRate(&emitter, 0.0f);  // desliga emissao continua
//
//  Render usa Camera_GetActive() automaticamente, igual a Entity_Render.
// ---------------------------------------------------------------------------
typedef struct IXEmitter
{
    // --- Buffer ---
    IXParticle* particles;
    int         capacity;
    int         activeCount;   // quantas estao vivas

    // --- Posicao do emissor ---
    float x, y;
    float spreadX, spreadY;    // variacao de posicao no spawn (raio)

    // --- Spawn rate ---
    float spawnRate;           // particulas por segundo (0 = manual/burst)
    float spawnAccum;          // acumulador fracional

    // --- Parametros de particula ---
    float lifeMin, lifeMax;    // segundos
    float vxMin, vxMax;        // velocidade horizontal
    float vyMin, vyMax;        // velocidade vertical
    float ax, ay;              // aceleracao (gravidade, vento)
    int   sizeStart;           // tamanho no nascimento
    int   sizeEnd;             // tamanho na morte
    unsigned int colorStart;   // ARGB no nascimento
    unsigned int colorEnd;     // ARGB na morte (alpha 0 = some ao morrer)

    // --- Textura (NULL = quadrado solido) ---
    IDirect3DTexture9* texture;
    int texFrameW, texFrameH;  // 0 = textura inteira

    // --- Flags ---
    int active;    // 0 = Update nao spawna (mas continua simulando as vivas)
    int paused;    // 1 = nem simula nem spawna

} IXEmitter;

// ---------------------------------------------------------------------------
//  Inicializacao
// ---------------------------------------------------------------------------
void Emitter_Init(IXEmitter* e, IXParticle* buffer, int capacity);

// ---------------------------------------------------------------------------
//  Configuracao
// ---------------------------------------------------------------------------
void Emitter_SetPosition(IXEmitter* e, float x, float y);
void Emitter_SetSpread(IXEmitter* e, float spreadX, float spreadY);
void Emitter_SetSpawnRate(IXEmitter* e, float particlesPerSecond);
void Emitter_SetLifetime(IXEmitter* e, float minSec, float maxSec);
void Emitter_SetVelocity(IXEmitter* e,
                          float vxMin, float vxMax,
                          float vyMin, float vyMax);
void Emitter_SetGravity(IXEmitter* e, float ax, float ay);
void Emitter_SetSize(IXEmitter* e, int sizeStart, int sizeEnd);
void Emitter_SetColorOverLife(IXEmitter* e,
                               unsigned int colorStart,
                               unsigned int colorEnd);
void Emitter_SetTexture(IXEmitter* e, IDirect3DTexture9* texture,
                         int frameW, int frameH);

// ---------------------------------------------------------------------------
//  Controle
// ---------------------------------------------------------------------------
// Dispara N particulas de uma vez (independente do spawnRate).
void Emitter_Burst(IXEmitter* e, int count);

// Para de spawnar novas (as vivas continuam ate morrer).
void Emitter_Stop(IXEmitter* e);

// Reinicia: mata todas as particulas e reseta acumulador.
void Emitter_Reset(IXEmitter* e);

void Emitter_Pause(IXEmitter* e);
void Emitter_Resume(IXEmitter* e);

// ---------------------------------------------------------------------------
//  Logica e rendering  (chamar uma vez por frame)
// ---------------------------------------------------------------------------
void Emitter_Update(IXEmitter* e, float deltaSeconds);
void Emitter_Render(const IXEmitter* e);

// ---------------------------------------------------------------------------
//  Queries
// ---------------------------------------------------------------------------
int Emitter_GetActiveCount(const IXEmitter* e);
int Emitter_IsEmpty(const IXEmitter* e);       // 1 se nenhuma particula viva

#ifdef __cplusplus
}
#endif

#endif // IX_CORE_PARTICLES_H
