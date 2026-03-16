#ifndef IX_CORE_SPLASHSCREEN_H
#define IX_CORE_SPLASHSCREEN_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------
//  InteractiveX r0.2 -- Splash Screen
//
//  Exibe uma imagem de tela cheia com fade in/out antes do jogo comecar.
//  Deve ser chamado APOS Engine_Initialize e ANTES do loop principal.
//  Roda seu proprio mini message-loop — a janela permanece responsiva.
//
//  Imagem:
//    - Se 'imagePath' for NULL, tenta carregar "assets/splash/bg.png".
//    - Redimensionada para cobrir a tela mantendo proporcao (cover).
//    - Suporta PNG e BMP (via GDI+, que ja e usado pelo Image loader).
//
//  Saida antecipada:
//    - ESC, Enter, Space ou clique do mouse encerra o splash imediatamente
//      (com fade out acelerado se skipFade=0, ou instantaneo se skipFade=1).
//
//  Uso rapido:
//      Splashscreen_Show(hWnd, NULL, 3.0f);   // 3 segundos, fade padrao
//
//  Uso completo:
//      Splashscreen_ShowEx(hWnd,
//          "assets/logo.png",
//          4.0f,            // duracao total em segundos
//          0.6f,            // fade in
//          0.8f,            // fade out
//          0xFF000000,      // cor de fundo (ARGB)
//          1);              // skipFade=1: ESC/clique encerra sem fade out
// ---------------------------------------------------------------------------

// Exibe splash com fade in/out padrao (0.5s cada) e fundo preto.
// imagePath NULL = "assets/splash/bg.png".
void Splashscreen_Show(HWND hWnd, const char* imagePath, float durationSeconds);

// Controle total sobre o splash screen.
void Splashscreen_ShowEx(HWND hWnd,
                          const char* imagePath,
                          float durationSeconds,
                          float fadeInSeconds,
                          float fadeOutSeconds,
                          unsigned int bgColor,
                          int skipFadeOnExit);

#ifdef __cplusplus
}
#endif

#endif // IX_CORE_SPLASHSCREEN_H
