#ifndef IX_EDITOR_H
#define IX_EDITOR_H

int IXEditor_Initialize();
void IXEditor_Shutdown();
void IXEditor_Update();

// "Botões" de comando do editor.
void IXEditor_ButtonAddCube(const char* name);
void IXEditor_ButtonMoveObject(int objectId, float dx, float dy, float dz);
void IXEditor_ButtonResizeObject(int objectId, float sx, float sy, float sz);
void IXEditor_ButtonRenameObject(int objectId, const char* newName);
void IXEditor_ButtonDeleteObject(int objectId);
void IXEditor_ButtonSelectObject(int objectId);

// Importação simples de texturas.
int  IXEditor_ButtonImportTexture(const char* filePath, const char* textureName);
void IXEditor_ButtonAssignTextureToObject(int objectId, int textureId);
void IXEditor_DrawTextureLibrary();

// Painéis/visualização.
void IXEditor_DrawHierarchy();
void IXEditor_DrawInspector(int objectId);

#endif // IX_EDITOR_H
