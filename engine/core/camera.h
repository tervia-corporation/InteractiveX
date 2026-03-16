#ifndef IX_CORE_CAMERA_H
#define IX_CORE_CAMERA_H

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------
//  IXCamera
//
//  Transforma coordenadas de mundo em coordenadas de tela.
//  Suporta posicao, zoom, limites do mundo e screen shake.
//
//  Uso tipico:
//
//      // inicializa uma vez
//      IXCamera cam;
//      Camera_Init(&cam);
//      Camera_SetWorldBounds(&cam, 0, 0, 2000, 1500);
//
//      // a cada frame — update antes de renderizar
//      Camera_Follow(&cam, player.x, player.y, Timer_GetDeltaSeconds());
//      Camera_Update(&cam, Timer_GetDeltaSeconds());   // processa shake
//
//      // rendering com camera
//      Camera_Attach(&cam);
//          EntityList_RenderAll(&world);    // Entity_Render ja usa a camera
//      Camera_Detach();
//
//      // ou manualmente:
//      int sx, sy;
//      Camera_WorldToScreen(&cam, entity.x, entity.y, &sx, &sy);
//      Renderer_DrawSprite(tex, 0,0,0,0, sx, sy, w, h, 0xFFFFFFFF);
//
//  Frustum culling manual:
//      if (Camera_IsVisible(&cam, entity.x, entity.y, entity.w, entity.h))
//          Entity_Render(&entity);
// ---------------------------------------------------------------------------

typedef struct IXCamera
{
    // --- Posicao no mundo (canto top-left do viewport) ---
    float x, y;

    // --- Zoom (1.0 = normal, 2.0 = dobrado, 0.5 = metade) ---
    float zoom;

    // --- Limites do mundo  (Camera_SetWorldBounds) ---
    // Se boundsEnabled == 0, a camera pode ir para qualquer posicao.
    int   boundsEnabled;
    float boundsX, boundsY;
    float boundsW, boundsH;

    // --- Follow suave (Camera_Follow) ---
    // lerp: 0.0 = nao segue, 1.0 = snap instantaneo. ~0.08 e suave.
    float followLerp;

    // --- Screen shake ---
    float shakeIntensity;   // intensidade atual (pixels, pre-zoom)
    float shakeDuration;    // tempo restante em segundos
    float shakeOffsetX;     // offset calculado neste frame
    float shakeOffsetY;

    // --- Tamanho do viewport (atualizado por Camera_Update) ---
    int viewportW, viewportH;

} IXCamera;

// ---------------------------------------------------------------------------
//  Inicializacao
// ---------------------------------------------------------------------------
void Camera_Init(IXCamera* cam);

// ---------------------------------------------------------------------------
//  Configuracao
// ---------------------------------------------------------------------------
void Camera_SetPosition(IXCamera* cam, float x, float y);
void Camera_SetZoom(IXCamera* cam, float zoom);

// Define os limites do mundo. A camera nunca mostrara alem deles.
// Passe w=0 e h=0 para desabilitar.
void Camera_SetWorldBounds(IXCamera* cam,
                            float x, float y, float w, float h);

// Velocidade de interpolacao do Camera_Follow (0.0 - 1.0).
// Padrao: 0.08. Valores maiores = mais rigido.
void Camera_SetFollowLerp(IXCamera* cam, float lerp);

// ---------------------------------------------------------------------------
//  Logica  (chamar uma vez por frame, antes de renderizar)
// ---------------------------------------------------------------------------

// Move a camera suavemente em direcao a (targetX, targetY) no espaco de mundo.
// O alvo fica centralizado no viewport.
void Camera_Follow(IXCamera* cam, float targetX, float targetY, float deltaSeconds);

// Snap instantaneo: centraliza a camera em (targetX, targetY) sem lerp.
void Camera_CenterOn(IXCamera* cam, float targetX, float targetY);

// Processa shake e atualiza viewport. Deve ser chamado todo frame.
void Camera_Update(IXCamera* cam, float deltaSeconds);

// Dispara um screen shake.
// intensity  amplitude maxima em pixels (pre-zoom)
// duration   duracao em segundos
void Camera_Shake(IXCamera* cam, float intensity, float duration);

// ---------------------------------------------------------------------------
//  Transformacoes
// ---------------------------------------------------------------------------

// Converte posicao de mundo -> posicao de tela.
void Camera_WorldToScreen(const IXCamera* cam,
                           float worldX, float worldY,
                           int* screenX, int* screenY);

// Converte posicao de tela -> posicao de mundo.
void Camera_ScreenToWorld(const IXCamera* cam,
                           int screenX, int screenY,
                           float* worldX, float* worldY);

// Retorna 1 se o retangulo de mundo (x,y,w,h) e visivel no viewport atual.
// Util para frustum culling manual.
int Camera_IsVisible(const IXCamera* cam,
                     float worldX, float worldY, int w, int h);

// ---------------------------------------------------------------------------
//  Attach / Detach
//
//  Camera_Attach define a camera global que Entity_Render usa para
//  transformar coordenadas de mundo em tela automaticamente.
//  Camera_Detach remove essa camera global (Entity_Render volta a usar
//  coordenadas de tela diretamente, como antes).
//
//  Apenas uma camera pode estar ativa por vez.
// ---------------------------------------------------------------------------
void Camera_Attach(IXCamera* cam);
void Camera_Detach();

// Retorna a camera global ativa, ou NULL se nenhuma estiver attached.
IXCamera* Camera_GetActive();

#ifdef __cplusplus
}
#endif

#endif // IX_CORE_CAMERA_H
