#include <pch.h>
#include "Application/Application.h"

#ifdef _WIN32
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

int main()
{
	SliceBuild::Application app;
	app.Init();
	app.Run();
	app.Exit();
	return 0;
}
