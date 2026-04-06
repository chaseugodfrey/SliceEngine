#include <pch.h>
#include "Application/Application.h"
#include <Core/Core.h>

#ifdef _WIN32
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

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

	SliceBuild::Application app;
	app.Init();

	if (smokeTest)
	{
		SLICE_LOG("Smoke test successful. Exiting game.");
		app.Exit();
		return 0;
	}

	app.Run();
	app.Exit();
	return 0;
}
