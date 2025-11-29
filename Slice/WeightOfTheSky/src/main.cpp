#include <pch.h>
#include "Application/Application.h"

int main()
{
	SliceBuild::Application app;
	app.Init();
	app.Run();
	app.Exit();
	return 0;
}
