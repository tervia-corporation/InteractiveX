#ifndef IX_CORE_SAVELOAD_H
#define IX_CORE_SAVELOAD_H

// ---------------------------------------------------------------------------
//  InteractiveX r0.2 -- Save/Load binario simples
//
//  O jogo escolhe onde salvar chamando SaveLoad_SetBasePath() antes de
//  qualquer leitura/escrita.  Se nao chamar, o padrao e o diretorio do
//  executavel.
//
//  Caminhos comuns (Windows):
//    %APPDATA%\NomeDoJogo\          -> SaveLoad_SetBasePath(IX_SAVE_PATH_APPDATA, "NomeDoJogo")
//    Documentos\NomeDoJogo\data\    -> SaveLoad_SetBasePath(IX_SAVE_PATH_DOCUMENTS, "NomeDoJogo\\data")
//    Pasta do executavel\save\      -> SaveLoad_SetBasePath(IX_SAVE_PATH_EXE, "save")
//    Caminho absoluto livre         -> SaveLoad_SetBasePathAbsolute("C:\\meu\\caminho")
//
//  Formato do arquivo .ixsav:
//    [4 bytes] magic   = 0x49585356 ("IXSV")
//    [4 bytes] version = numero de versao passado pelo jogo (ex: 1)
//    [4 bytes] count   = numero de entradas
//    Para cada entrada:
//      [2 bytes] key len  (sem nulo)
//      [N bytes] key
//      [4 bytes] data len
//      [M bytes] data
// ---------------------------------------------------------------------------

#define IX_SAVE_MAGIC   0x49585356u   /* "IXSV" */

// Onde a pasta-base fica ancorada
#define IX_SAVE_PATH_EXE        0   // ao lado do executavel  (padrao)
#define IX_SAVE_PATH_APPDATA    1   // %APPDATA%\<subdir>
#define IX_SAVE_PATH_DOCUMENTS  2   // Documentos\<subdir>

// ---------------------------------------------------------------------------
//  Configuracao de caminho
// ---------------------------------------------------------------------------

// Define base a partir de uma raiz conhecida + subdiretorio relativo.
// subdir pode conter barras, ex: "MeuJogo\\saves"
// Cria o diretorio se nao existir.
void SaveLoad_SetBasePath(int root, const char* subdir);

// Define um caminho absoluto livre (ex: "C:\\MeuJogo\\data").
// Cria o diretorio se nao existir.
void SaveLoad_SetBasePathAbsolute(const char* absolutePath);

// Retorna o caminho base atual (somente leitura).
const char* SaveLoad_GetBasePath();

// ---------------------------------------------------------------------------
//  Escrita
// ---------------------------------------------------------------------------

typedef struct IXSaveWriter IXSaveWriter;

// Cria um writer em memoria.  version = numero de versao do jogo.
IXSaveWriter* SaveLoad_WriterCreate(unsigned int version);

// Escreve uma entrada de bytes brutos.
void SaveLoad_WriterPutBytes(IXSaveWriter* w, const char* key, const void* data, unsigned int len);

// Helpers tipados
void SaveLoad_WriterPutInt  (IXSaveWriter* w, const char* key, int value);
void SaveLoad_WriterPutFloat(IXSaveWriter* w, const char* key, float value);
void SaveLoad_WriterPutStr  (IXSaveWriter* w, const char* key, const char* str);

// Salva para <basePath>/<filename>.ixsav  (filename sem extensao ou com).
// Retorna 1 em sucesso, 0 em falha.
int  SaveLoad_WriterSave(IXSaveWriter* w, const char* filename);

// Libera o writer.
void SaveLoad_WriterDestroy(IXSaveWriter* w);

// ---------------------------------------------------------------------------
//  Leitura
// ---------------------------------------------------------------------------

typedef struct IXSaveReader IXSaveReader;

// Carrega <basePath>/<filename>.ixsav.
// Retorna NULL se o arquivo nao existir ou estiver corrompido.
IXSaveReader* SaveLoad_ReaderOpen(const char* filename);

// Versao gravada no arquivo.
unsigned int SaveLoad_ReaderGetVersion(IXSaveReader* r);

// Numero de entradas.
unsigned int SaveLoad_ReaderGetCount(IXSaveReader* r);

// Retorna ponteiro para os dados da chave (NULL se nao encontrar).
// *outLen recebe o tamanho em bytes.
const void*  SaveLoad_ReaderGetBytes(IXSaveReader* r, const char* key, unsigned int* outLen);

// Helpers tipados  (retornam 0/0.0f/"" se a chave nao existir)
int         SaveLoad_ReaderGetInt  (IXSaveReader* r, const char* key);
float       SaveLoad_ReaderGetFloat(IXSaveReader* r, const char* key);
// Copia ate bufLen-1 caracteres + nulo.  Retorna 1 se encontrou.
int         SaveLoad_ReaderGetStr  (IXSaveReader* r, const char* key, char* buf, unsigned int bufLen);

// Libera o reader.
void SaveLoad_ReaderDestroy(IXSaveReader* r);

// ---------------------------------------------------------------------------
//  Utilitarios
// ---------------------------------------------------------------------------

// Retorna 1 se o arquivo existir no caminho base.
int SaveLoad_FileExists(const char* filename);

// Remove o arquivo do caminho base.  Retorna 1 em sucesso.
int SaveLoad_FileDelete(const char* filename);

#endif // IX_CORE_SAVELOAD_H
