#include "editor.h"

int main()
{
    if (!IXEditor_Initialize()) return 1;

    IXEditor_ButtonAddCube("Cube_C");
    IXEditor_ButtonMoveObject(2, 1.0f, 0.5f, -0.25f);
    IXEditor_ButtonResizeObject(3, 2.0f, 1.0f, 0.5f);
    IXEditor_ButtonRenameObject(3, "Cube_B_Resized");

    int t1 = IXEditor_ButtonImportTexture("assets/textures/bricks.png", "Bricks");
    int t2 = IXEditor_ButtonImportTexture("assets/textures/metal.jpg", "Metal");
    IXEditor_ButtonAssignTextureToObject(2, t1);
    IXEditor_ButtonAssignTextureToObject(3, t2);

    IXEditor_ButtonSelectObject(3);
    IXEditor_Update();

    IXEditor_ButtonDeleteObject(4);
    IXEditor_ButtonSelectObject(2);
    IXEditor_Update();

    IXEditor_Shutdown();
    return 0;
}
