#ifndef IX_CORE_VIDEO_H
#define IX_CORE_VIDEO_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------
//  InteractiveX r0.2 -- Opcoes de video
//
//  Controla resolucao, fullscreen, vsync e bloqueio de redimensionamento.
//  Tudo e aplicado de uma vez com Video_Apply() — sem efeitos parciais.
//
//  Uso tipico (logo apos Engine_Initialize):
//
//      Video_Initialize(hWnd);
//      Video_SetResolution(1280, 720);
//      Video_SetVSync(1);
//      Video_LockResize(1);
//      Video_Apply();
//
//  Alteracao em runtime (ex: opcoes de video no menu):
//
//      Video_SetFullscreen(1);
//      Video_Apply();
//
//  Fullscreen usa modo borderless windowed (janela sem borda, tamanho do
//  monitor) — compativel com alt-tab e multiplos monitores.
// ---------------------------------------------------------------------------

// Modo de video disponivel (enumerado do adaptador)
typedef struct IXVideoMode
{
    int width;
    int height;
    int refreshRate;
} IXVideoMode;

// ---------------------------------------------------------------------------
//  Lifecycle
// ---------------------------------------------------------------------------

// Deve ser chamado apos Engine_Initialize, antes do loop principal.
void Video_Initialize(HWND hWnd);

// ---------------------------------------------------------------------------
//  Setters (nao tem efeito imediato — chame Video_Apply() depois)
// ---------------------------------------------------------------------------

// 1 = fullscreen borderless, 0 = janela normal (default: 0)
void Video_SetFullscreen(int fullscreen);

// Resolucao da janela em modo windowed (default: tamanho atual da janela)
// Ignorada em fullscreen (usa resolucao do monitor)
void Video_SetResolution(int width, int height);

// 1 = VSync ativado, 0 = imediato (default: 0)
void Video_SetVSync(int vsync);

// 1 = usuario nao pode redimensionar a janela, 0 = livre (default: 0)
void Video_LockResize(int locked);

// Resolucao minima permitida quando resize nao esta bloqueado
// (default: 640 x 480)
void Video_SetMinSize(int minWidth, int minHeight);

// ---------------------------------------------------------------------------
//  Aplicacao
// ---------------------------------------------------------------------------

// Aplica todas as configuracoes pendentes.
// Pode resetar o device D3D — chame fora do BeginFrame/EndFrame.
// Retorna 1 em sucesso, 0 em falha.
int Video_Apply();

// ---------------------------------------------------------------------------
//  Leitura
// ---------------------------------------------------------------------------

int Video_IsFullscreen();
int Video_IsVSync();
int Video_IsResizeLocked();
void Video_GetResolution(int* width, int* height);

// Preenche 'outModes' com os modos disponíveis no monitor principal.
// Retorna o numero de modos encontrados (ate maxCount).
// Filtra duplicatas e modos menores que 640x480.
int Video_GetAvailableModes(IXVideoMode* outModes, int maxCount);

// ---------------------------------------------------------------------------
//  Integracao com main.cpp (chamado internamente pelo MsgProc)
// ---------------------------------------------------------------------------

// Processa WM_GETMINMAXINFO respeitando LockResize e minSize.
// Retorne 0 do MsgProc depois de chamar esta funcao.
void Video_HandleGetMinMaxInfo(LPARAM lParam);

// Processa WM_SIZING (bloqueia resize se LockResize=1).
void Video_HandleSizing(WPARAM wParam, LPARAM lParam);

#ifdef __cplusplus
}
#endif

#endif // IX_CORE_VIDEO_H
