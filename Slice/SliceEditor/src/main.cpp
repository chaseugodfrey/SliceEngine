#include <pch.h>
#include "Editor.h"

int main()
{
	SliceEditor::Editor editor;
	editor.Init();
	editor.Run();
	editor.Exit();
	return 0;
}
