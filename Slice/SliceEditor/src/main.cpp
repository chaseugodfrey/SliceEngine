#include <pch.h>
#include "Editor.h"
#include <Core/Core.h>

int main(int argc, char** argv)
{
	bool smokeTest = false;
	for (int i = 1; i < argc; ++i)
	{
		if (std::string(argv[i]) == "--smoke-test")
		{
			smokeTest = true;
		}
	}

	if (smokeTest)
	{
		SliceEngine::Core::GetInstance()->GetWindowManager()->SetSmokeTest(true);
	}

	SliceEditor::Editor editor;
	editor.Init();

	if (smokeTest)
	{
		SLICE_LOG("Smoke test successful. Exiting editor.");
		editor.Exit();
		return 0;
	}

	editor.Run();
	editor.Exit();
	return 0;
}
