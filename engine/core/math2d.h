#ifndef IX_CORE_MATH2D_H
#define IX_CORE_MATH2D_H

// ---------------------------------------------------------------------------
//  InteractiveX r0.2 — Math2D
//
//  Header-only (sem .cpp). Inclua onde precisar.
//  Todas as funcoes sao inline — sem overhead de chamada.
//
//  Tipos:
//      IXVec2   — vetor 2D (float x, y)
//
//  Grupos de funcoes:
//      Vec2_*        — operacoes sobre vetores
//      IX_Lerp*      — interpolacao linear (float, Vec2)
//      IX_Clamp*     — limitacao de valores
//      IX_Angle*     — angulos em radianos e graus
//      IX_Overlap*   — testes de sobreposicao AABB / circulo
//      IX_Wrap*      — wrap de valores em range
//      IX_Sign / IX_Abs / IX_Min / IX_Max
//
//  Uso tipico:
//      IXVec2 dir = Vec2_Sub(target, player);
//      float  dist = Vec2_Length(dir);
//      IXVec2 norm = Vec2_Normalize(dir);
//      player.x += norm.x * speed * dt;
//      player.y += norm.y * speed * dt;
//
//      float angle = Vec2_Angle(dir);  // radianos
//      IXVec2 rotated = Vec2_Rotate(dir, IX_DEG2RAD(45.0f));
// ---------------------------------------------------------------------------

#include <math.h>  // sqrtf, atan2f, cosf, sinf, fabsf

// ---------------------------------------------------------------------------
//  Constantes
// ---------------------------------------------------------------------------
#define IX_PI       3.14159265358979323846f
#define IX_TAU      6.28318530717958647692f
#define IX_DEG2RAD(d) ((d) * (IX_PI / 180.0f))
#define IX_RAD2DEG(r) ((r) * (180.0f / IX_PI))

// ---------------------------------------------------------------------------
//  IXVec2
// ---------------------------------------------------------------------------
typedef struct IXVec2 { float x, y; } IXVec2;

// Construtores
static inline IXVec2 Vec2(float x, float y)          { IXVec2 v; v.x = x; v.y = y; return v; }
static inline IXVec2 Vec2_Zero()                      { return Vec2(0.0f, 0.0f); }
static inline IXVec2 Vec2_One()                       { return Vec2(1.0f, 1.0f); }
static inline IXVec2 Vec2_Up()                        { return Vec2(0.0f, -1.0f); } // Y+ = baixo em screen space
static inline IXVec2 Vec2_Down()                      { return Vec2(0.0f,  1.0f); }
static inline IXVec2 Vec2_Left()                      { return Vec2(-1.0f, 0.0f); }
static inline IXVec2 Vec2_Right()                     { return Vec2( 1.0f, 0.0f); }

// Aritmetica
static inline IXVec2 Vec2_Add(IXVec2 a, IXVec2 b)    { return Vec2(a.x + b.x, a.y + b.y); }
static inline IXVec2 Vec2_Sub(IXVec2 a, IXVec2 b)    { return Vec2(a.x - b.x, a.y - b.y); }
static inline IXVec2 Vec2_Scale(IXVec2 v, float s)   { return Vec2(v.x * s, v.y * s); }
static inline IXVec2 Vec2_Neg(IXVec2 v)               { return Vec2(-v.x, -v.y); }
static inline IXVec2 Vec2_Mul(IXVec2 a, IXVec2 b)    { return Vec2(a.x * b.x, a.y * b.y); }

// Magnitude
static inline float  Vec2_LengthSq(IXVec2 v)         { return v.x * v.x + v.y * v.y; }
static inline float  Vec2_Length(IXVec2 v)            { return sqrtf(Vec2_LengthSq(v)); }

// Normalizacao (retorna Vec2_Zero se comprimento < epsilon)
static inline IXVec2 Vec2_Normalize(IXVec2 v)
{
    float len = Vec2_Length(v);
    if (len < 0.000001f) return Vec2_Zero();
    return Vec2(v.x / len, v.y / len);
}

// Dot / Cross (escalar 2D)
static inline float  Vec2_Dot(IXVec2 a, IXVec2 b)    { return a.x * b.x + a.y * b.y; }
static inline float  Vec2_Cross(IXVec2 a, IXVec2 b)  { return a.x * b.y - a.y * b.x; }

// Distancia entre dois pontos
static inline float  Vec2_Dist(IXVec2 a, IXVec2 b)   { return Vec2_Length(Vec2_Sub(b, a)); }
static inline float  Vec2_DistSq(IXVec2 a, IXVec2 b) { return Vec2_LengthSq(Vec2_Sub(b, a)); }

// Angulo do vetor em radianos (0 = direita, anti-horario)
// Retorna [-PI, PI]
static inline float  Vec2_Angle(IXVec2 v)             { return atan2f(v.y, v.x); }

// Angulo entre dois vetores (em radianos)
static inline float  Vec2_AngleBetween(IXVec2 a, IXVec2 b)
{
    float dot = Vec2_Dot(Vec2_Normalize(a), Vec2_Normalize(b));
    if (dot >  1.0f) dot =  1.0f;
    if (dot < -1.0f) dot = -1.0f;
    return acosf(dot);
}

// Cria vetor a partir de angulo (radianos) e comprimento
static inline IXVec2 Vec2_FromAngle(float radians, float length)
{
    return Vec2(cosf(radians) * length, sinf(radians) * length);
}

// Rotaciona vetor em torno da origem
static inline IXVec2 Vec2_Rotate(IXVec2 v, float radians)
{
    float c = cosf(radians), s = sinf(radians);
    return Vec2(v.x * c - v.y * s, v.x * s + v.y * c);
}

// Rotaciona ponto em torno de um pivot
static inline IXVec2 Vec2_RotateAround(IXVec2 point, IXVec2 pivot, float radians)
{
    return Vec2_Add(pivot, Vec2_Rotate(Vec2_Sub(point, pivot), radians));
}

// Reflexao: reflete v em torno da normal n (n deve estar normalizado)
static inline IXVec2 Vec2_Reflect(IXVec2 v, IXVec2 normal)
{
    float d = 2.0f * Vec2_Dot(v, normal);
    return Vec2_Sub(v, Vec2_Scale(normal, d));
}

// Perpendicular (rotacao de 90 graus anti-horario)
static inline IXVec2 Vec2_Perp(IXVec2 v)              { return Vec2(-v.y, v.x); }

// Lerp entre dois vetores
static inline IXVec2 Vec2_Lerp(IXVec2 a, IXVec2 b, float t)
{
    return Vec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
}

// Move em direcao a target sem ultrapassar maxDist
static inline IXVec2 Vec2_MoveTowards(IXVec2 current, IXVec2 target, float maxDist)
{
    IXVec2 delta = Vec2_Sub(target, current);
    float dist   = Vec2_Length(delta);
    if (dist <= maxDist || dist < 0.000001f) return target;
    return Vec2_Add(current, Vec2_Scale(Vec2_Normalize(delta), maxDist));
}

// Clamp de comprimento
static inline IXVec2 Vec2_ClampLength(IXVec2 v, float maxLength)
{
    float len = Vec2_Length(v);
    if (len <= maxLength || len < 0.000001f) return v;
    return Vec2_Scale(Vec2_Normalize(v), maxLength);
}

// ---------------------------------------------------------------------------
//  Escalares uteis
// ---------------------------------------------------------------------------
static inline float IX_Lerp(float a, float b, float t)     { return a + (b - a) * t; }
static inline float IX_Clamp(float v, float lo, float hi)  { return v < lo ? lo : (v > hi ? hi : v); }
static inline float IX_Clamp01(float v)                    { return IX_Clamp(v, 0.0f, 1.0f); }
static inline float IX_Abs(float v)                        { return fabsf(v); }
static inline float IX_Sign(float v)                       { return (v > 0.0f) ? 1.0f : (v < 0.0f) ? -1.0f : 0.0f; }
static inline float IX_Min(float a, float b)               { return a < b ? a : b; }
static inline float IX_Max(float a, float b)               { return a > b ? a : b; }
static inline float IX_Min3(float a, float b, float c)     { return IX_Min(a, IX_Min(b, c)); }
static inline float IX_Max3(float a, float b, float c)     { return IX_Max(a, IX_Max(b, c)); }

// Wrap: mantém v dentro de [lo, hi) ciclicamente
static inline float IX_Wrap(float v, float lo, float hi)
{
    float range = hi - lo;
    if (range <= 0.0f) return lo;
    while (v < lo)  v += range;
    while (v >= hi) v -= range;
    return v;
}

// Wrap de angulo para [-PI, PI]
static inline float IX_WrapAngle(float radians)            { return IX_Wrap(radians, -IX_PI, IX_PI); }

// Smooth step (suavizacao cubica 0..1)
static inline float IX_SmoothStep(float t)
{
    t = IX_Clamp01(t);
    return t * t * (3.0f - 2.0f * t);
}

// Smooth step entre dois valores
static inline float IX_SmoothLerp(float a, float b, float t)  { return IX_Lerp(a, b, IX_SmoothStep(t)); }

// Aproxima v em direcao a target sem ultrapassar maxStep
static inline float IX_MoveTowards(float v, float target, float maxStep)
{
    float diff = target - v;
    if (IX_Abs(diff) <= maxStep) return target;
    return v + IX_Sign(diff) * maxStep;
}

// ---------------------------------------------------------------------------
//  Testes de sobreposicao
// ---------------------------------------------------------------------------

// AABB vs AABB (x,y = canto superior esquerdo, w/h = dimensoes)
static inline int IX_OverlapAABB(float ax, float ay, float aw, float ah,
                                   float bx, float by, float bw, float bh)
{
    return (ax < bx + bw && ax + aw > bx &&
            ay < by + bh && ay + ah > by) ? 1 : 0;
}

// Circulo vs Circulo
static inline int IX_OverlapCircle(float ax, float ay, float ar,
                                    float bx, float by, float br)
{
    float dx = bx - ax, dy = by - ay, r = ar + br;
    return (dx * dx + dy * dy) < (r * r) ? 1 : 0;
}

// Ponto dentro de AABB
static inline int IX_PointInAABB(float px, float py,
                                   float rx, float ry, float rw, float rh)
{
    return (px >= rx && px < rx + rw && py >= ry && py < ry + rh) ? 1 : 0;
}

// Ponto dentro de circulo
static inline int IX_PointInCircle(float px, float py,
                                    float cx, float cy, float cr)
{
    float dx = px - cx, dy = py - cy;
    return (dx * dx + dy * dy) < (cr * cr) ? 1 : 0;
}

// Ray vs AABB — retorna 1 se houve colisao, preenche *tHit (0..1 ao longo do ray)
// ray: origem (ox,oy) + direcao normalizada (dx,dy) + comprimento maxLen
static inline int IX_RayAABB(float ox, float oy, float dx, float dy, float maxLen,
                               float rx, float ry, float rw, float rh,
                               float* tHit)
{
    float tmin = 0.0f, tmax = maxLen;
    // Eixo X
    if (dx != 0.0f) {
        float t1 = (rx       - ox) / dx;
        float t2 = (rx + rw  - ox) / dx;
        if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
        tmin = IX_Max(tmin, t1);
        tmax = IX_Min(tmax, t2);
    } else if (ox < rx || ox > rx + rw) return 0;
    // Eixo Y
    if (dy != 0.0f) {
        float t1 = (ry       - oy) / dy;
        float t2 = (ry + rh  - oy) / dy;
        if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
        tmin = IX_Max(tmin, t1);
        tmax = IX_Min(tmax, t2);
    } else if (oy < ry || oy > ry + rh) return 0;
    if (tmax < tmin) return 0;
    if (tHit) *tHit = tmin;
    return 1;
}

#endif // IX_CORE_MATH2D_H
