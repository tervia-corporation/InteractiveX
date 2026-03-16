#ifndef IX_CORE_RANDOM_H
#define IX_CORE_RANDOM_H

// ---------------------------------------------------------------------------
//  InteractiveX r0.2 -- RNG publico reproduzivel
//
//  Header-only (sem .cpp). Inclua onde precisar.
//  Todas as funcoes sao inline.
//
//  Uso basico:
//      IXRng rng;
//      Rng_Seed(&rng, 12345);
//
//      float f  = Rng_Float(&rng, -1.0f, 1.0f);  // [-1, 1)
//      int   n  = Rng_Int  (&rng, 1, 6);          // [1, 6]  (dado de 6 faces)
//      int   ok = Rng_Chance(&rng, 0.25f);         // 25% de chance
//
//  Reproducibilidade:
//      A mesma seed produz a mesma sequencia de numeros em qualquer plataforma.
//      Util para geracao procedural, replays e testes deterministicos.
//
//  Algoritmo:
//      LCG (Linear Congruential Generator) com os parametros de Numerical Recipes:
//          estado = estado * 1664525 + 1013904223   (mod 2^32)
//      Simples, rapido, suficiente para jogos. Nao criptograficamente seguro.
//
//  Cada instancia IXRng tem seu proprio estado -- varios geradores podem
//  coexistir sem interferencia (ex: um para gameplay, outro para VFX).
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Tipo
// ---------------------------------------------------------------------------
typedef struct IXRng
{
    unsigned int state;
} IXRng;

// ---------------------------------------------------------------------------
//  Semente
// ---------------------------------------------------------------------------

// Inicializa o gerador com uma seed.
// Seed 0 e valida (e diferente de seed 1).
static inline void Rng_Seed(IXRng* rng, unsigned int seed)
{
    // Aplica um mix inicial para que seeds proximas nao gerem
    // sequencias similares no primeiro numero.
    // Baseado no hash de Thomas Wang (32-bit).
    seed = (seed ^ 61u) ^ (seed >> 16u);
    seed *= 9u;
    seed ^= seed >> 4u;
    seed *= 0x27D4EB2Du;
    seed ^= seed >> 15u;
    rng->state = seed ? seed : 1u;  // estado nunca pode ser 0
}

// ---------------------------------------------------------------------------
//  Geracao primitiva
// ---------------------------------------------------------------------------

// Avanca o estado e retorna um unsigned int de 32 bits sem restricao.
static inline unsigned int Rng_NextU32(IXRng* rng)
{
    rng->state = rng->state * 1664525u + 1013904223u;
    return rng->state;
}

// Retorna float em [0, 1)  usando os 16 bits superiores (mais qualidade que os inferiores em LCGs).
static inline float Rng_NextFloat01(IXRng* rng)
{
    return (float)(Rng_NextU32(rng) >> 16u) / 65536.0f;
}

// ---------------------------------------------------------------------------
//  Helpers de uso comum
// ---------------------------------------------------------------------------

// Float em [lo, hi)
static inline float Rng_Float(IXRng* rng, float lo, float hi)
{
    return lo + Rng_NextFloat01(rng) * (hi - lo);
}

// Inteiro em [lo, hi]  (inclusivo dos dois lados)
static inline int Rng_Int(IXRng* rng, int lo, int hi)
{
    if (lo >= hi) return lo;
    unsigned int range = (unsigned int)(hi - lo + 1);
    return lo + (int)(Rng_NextU32(rng) % range);
}

// Retorna 1 com probabilidade p (0.0 = nunca, 1.0 = sempre).
static inline int Rng_Chance(IXRng* rng, float p)
{
    return Rng_NextFloat01(rng) < p ? 1 : 0;
}

// Cara ou coroa: retorna 0 ou 1 com probabilidade igual.
static inline int Rng_Bool(IXRng* rng)
{
    return (int)(Rng_NextU32(rng) >> 31u);
}

// Retorna -1 ou +1 com probabilidade igual.
static inline int Rng_Sign(IXRng* rng)
{
    return Rng_Bool(rng) ? 1 : -1;
}

// Float sinalizado em (-1, 1)  -- util para jitter, offset, shake.
static inline float Rng_FloatSigned(IXRng* rng)
{
    return Rng_Float(rng, -1.0f, 1.0f);
}

// Escolhe um indice aleatorio em [0, count-1].
// Retorna 0 se count <= 0.
static inline int Rng_Index(IXRng* rng, int count)
{
    if (count <= 0) return 0;
    return Rng_Int(rng, 0, count - 1);
}

// Baralha um array de ints usando Fisher-Yates.
// arr: ponteiro para o array, count: numero de elementos.
static inline void Rng_ShuffleInts(IXRng* rng, int* arr, int count)
{
    for (int i = count - 1; i > 0; i--)
    {
        int j   = Rng_Int(rng, 0, i);
        int tmp = arr[i];
        arr[i]  = arr[j];
        arr[j]  = tmp;
    }
}

// ---------------------------------------------------------------------------
//  Utilitario: seed rapida a partir do tempo do sistema (nao deterministica)
// ---------------------------------------------------------------------------
#include <time.h>
static inline void Rng_SeedFromTime(IXRng* rng)
{
    Rng_Seed(rng, (unsigned int)time(NULL));
}

#endif // IX_CORE_RANDOM_H
