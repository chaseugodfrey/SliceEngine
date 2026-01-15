#ifndef MAIN
#define MAIN

#include "Font.h"

#if !COMPILE_ONLY || _DEBUG
#include "OpenGL_Starter.h"
#else

#include <json.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>
#endif


int main(int argc, char** argv)
{
#if COMPILE_ONLY && !_DEBUG

	std::cout << "Current directory: " << std::filesystem::current_path() << std::endl;

	if (!std::filesystem::exists(argv[0])) {
		std::cout << "File does not exist: " << argv[0] << std::endl;
		return -1;
	}
	std::fstream fs(argv[0]);
	if (!fs.good()) {
		std::cout << "Error reading metafile: " << argv[0] << std::endl;
		return -1;
	}

	nlohmann::json obj = nlohmann::json::parse(fs);

	/*
	* not actually sure what the keys are called yet
	*/

	if (!obj.contains("assetPath") || !obj.contains("resourcePath")) {
		std::cout << "Invalid metafile" << std::endl;
		return  -1;
	}

	std::string asset_path = obj["assetPath"];
	std::string resource_path = obj["resourcePath"];

	//current compiler switchs:
	/*
	* Font resolution, default = 50, min = 16, max = 200
	* Padding, default = 2
	*/
	Font::FontData font_compiler;
	if (obj.contains("fontReso")) {
		font_compiler.font_size = obj["fontReso"];
	}
	if (obj.contains("padding")) {
		font_compiler.padding = obj["padding"];
	}
	font_compiler.Load_TTF_File(asset_path);
	font_compiler.Save_Fnt_File(resource_path);
	//return compiler.Compile_Asset(asset_path.c_str(), resource_path.c_str(), obj);
#else

	OpenGL_Starter starter;

	starter.Init();
	starter.Update();
	starter.Terminate();

#endif
}	

#endif
