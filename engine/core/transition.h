#ifndef IX_CORE_TRANSITION_H
#define IX_CORE_TRANSITION_H

// ---------------------------------------------------------------------------
//  InteractiveX r0.2 -- Scene Transitions
//
//  Substitui Scene_RequestChange quando se quer uma transicao animada.
//
//  Uso basico:
//      // Fade simples para outra cena (0.4s):
//      Transition_FadeBlack(IX_SCENE_MENU, 0.4f);
//
//      // Controle completo:
//      Transition_Request(IX_SCENE_MENU, IX_TRANSITION_WIPE_H, 0.5f, 0x000000);
//
//  O jogo pode chamar Transition_IsActive() para saber se nao deve
//  processar input ou iniciar outra transicao.
//
//  Tipos disponiveis:
//      IX_TRANSITION_FADE    fade para cor solida e volta (smooth)
//      IX_TRANSITION_WIPE_H  barra horizontal: cobre da esq. p/ dir., descobre da dir. p/ esq.
//      IX_TRANSITION_WIPE_V  barra vertical: cobre de cima p/ baixo, descobre de baixo p/ cima
//
//  Integracao (ja feita internamente):
//      engine.cpp    chama Transition_Update() a cada frame
//      renderer.cpp  chama Transition_DrawOverlay() dentro de EndFrame
// ---------------------------------------------------------------------------

#define IX_TRANSITION_FADE   0
#define IX_TRANSITION_WIPE_H 1
#define IX_TRANSITION_WIPE_V 2

// ---------------------------------------------------------------------------
//  API publica
// ---------------------------------------------------------------------------

// Solicita transicao animada para outra cena.
// type      : IX_TRANSITION_*
// duration  : duracao total em segundos (metade = fade out, metade = fade in)
// color     : cor RGB do overlay (0x000000 = preto, 0xFFFFFF = branco, etc.)
//             O alpha e controlado pela engine -- nao precisa especificar.
void Transition_Request(int toScene, int type, float durationSeconds, unsigned int color);

// Atalhos para os casos mais comuns
void Transition_FadeBlack(int toScene, float durationSeconds);
void Transition_FadeWhite(int toScene, float durationSeconds);

// Retorna 1 enquanto uma transicao esta em andamento.
// Use para bloquear input ou impedir novas transicoes.
int  Transition_IsActive();

// ---------------------------------------------------------------------------
//  Chamadas internas da engine (nao chamar manualmente)
// ---------------------------------------------------------------------------
void Transition_Update(float deltaSeconds);
void Transition_DrawOverlay(int screenWidth, int screenHeight);

#endif // IX_CORE_TRANSITION_H
