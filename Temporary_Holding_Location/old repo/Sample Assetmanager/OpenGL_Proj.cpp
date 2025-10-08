#ifndef MAIN
#define MAIN

#include "OpenGL_Starter.h"
#include "AssetManager.h"
int main()
{
	OpenGL_Starter starter;

	//starter.Init();
	//starter.Update();
	//starter.Terminate();

	Asset::AssetManager test{};
	test.SetupDirectories();
	auto desc_file = test.CreateDescriptor("Assets/Axol.png");
	if (desc_file.has_value()) {
		test.CompileAsset(desc_file.value());
	}
}	

#endif
