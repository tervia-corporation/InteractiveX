#ifndef IX_MEDIA_VIDEO_PLAYER_H
#define IX_MEDIA_VIDEO_PLAYER_H

// ---------------------------------------------------------------------------
//  InteractiveX r0.2 -- Video Player
//
//  Decodifica e exibe arquivos .mpg (MPEG-1 video + MPEG audio layer 2).
//  Usa pl_mpeg (header-only, dominio publico) para decodificacao e
//  miniaudio (ja presente na engine) para saida de audio sincronizado.
//
//  Dependencia:
//      engine/third_party/pl_mpeg/pl_mpeg.h
//      Download: https://github.com/phoboslab/pl_mpeg
//      (arquivo unico, ~4000 linhas, dominio publico)
//
//  Uso tipico:
//      // Antes do loop — abre o video
//      if (!VideoPlayer_Open("assets/video/intro.mpg")) { ... }
//
//      // Dentro do loop de jogo (ex: numa cena de cutscene)
//      VideoPlayer_Update(Timer_GetDeltaSeconds());
//      Renderer_BeginFrame();
//          VideoPlayer_Render(0, 0, screenW, screenH);  // cover
//      Renderer_EndFrame();
//
//      if (VideoPlayer_IsFinished()) Scene_RequestChange(IX_SCENE_MENU);
//
//      // Ao sair da cena
//      VideoPlayer_Close();
//
//  Modo bloqueante (cutscene simples sem loop de jogo):
//      VideoPlayer_PlayBlocking(hWnd, "assets/video/intro.mpg", 1);
//      // retorna quando o video termina ou ESC/espaco e pressionado
//
//  Formatos suportados: MPEG-1 (.mpg / .mpeg)
//  Para converter com ffmpeg:
//      ffmpeg -i input.mp4 -c:v mpeg1video -c:a mp2 -q:v 5 output.mpg
// ---------------------------------------------------------------------------

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------
//  Lifecycle
// ---------------------------------------------------------------------------

// Abre e prepara um arquivo .mpg para reproducao.
// path: caminho absoluto ou relativo ao executavel.
// Retorna 1 em sucesso, 0 em falha.
int  VideoPlayer_Open(const char* path);

// Fecha e libera todos os recursos. Seguro chamar mesmo se nao aberto.
void VideoPlayer_Close();

// ---------------------------------------------------------------------------
//  Controle
// ---------------------------------------------------------------------------
void VideoPlayer_Play();
void VideoPlayer_Pause();
void VideoPlayer_Stop();    // volta ao inicio, pausa
void VideoPlayer_SetLoop(int loop);

// ---------------------------------------------------------------------------
//  Update / Render (chamar dentro do loop de jogo)
// ---------------------------------------------------------------------------

// Avanca a decodificacao de acordo com o tempo decorrido.
// Deve ser chamado uma vez por frame, antes de VideoPlayer_Render.
void VideoPlayer_Update(float deltaSeconds);

// Desenha o frame atual esticado para o retangulo (x, y, w, h).
// Chame entre Renderer_BeginFrame() e Renderer_EndFrame().
// Para cobrir a tela toda: VideoPlayer_Render(0, 0, screenW, screenH).
// Para manter proporcao (letterbox/pillarbox), use VideoPlayer_RenderFit.
void VideoPlayer_Render(int x, int y, int w, int h);

// Renderiza mantendo proporcao original do video dentro do retangulo dado.
// fillColor: cor ARGB das barras laterais/superior-inferior (0x00 = transparente).
void VideoPlayer_RenderFit(int x, int y, int w, int h, unsigned int fillColor);

// ---------------------------------------------------------------------------
//  Estado
// ---------------------------------------------------------------------------
int   VideoPlayer_IsOpen();
int   VideoPlayer_IsPlaying();
int   VideoPlayer_IsPaused();
int   VideoPlayer_IsFinished();
float VideoPlayer_GetDuration();   // duracao total em segundos
float VideoPlayer_GetPosition();   // posicao atual em segundos
int   VideoPlayer_GetVideoWidth();
int   VideoPlayer_GetVideoHeight();

// ---------------------------------------------------------------------------
//  Volume do audio do video (independente do canal de musica)
// ---------------------------------------------------------------------------
void  VideoPlayer_SetVolume(float volume);  // 0.0 - 1.0
float VideoPlayer_GetVolume();

// ---------------------------------------------------------------------------
//  Modo bloqueante (cutscene sem precisar de loop de jogo)
//  Roda seu proprio mini message-loop. ESC/Espaco/Enter/Clique encerram.
//  overlay=1 mantem o overlay do debug, 0 oculta (recomendado para cutscene).
// ---------------------------------------------------------------------------
void VideoPlayer_PlayBlocking(HWND hWnd, const char* path,
                               int loop, int hideOverlay);

// ---------------------------------------------------------------------------
//  Diagnostico
// ---------------------------------------------------------------------------
const char* VideoPlayer_GetLastError();

#ifdef __cplusplus
}
#endif

#endif // IX_MEDIA_VIDEO_PLAYER_H
