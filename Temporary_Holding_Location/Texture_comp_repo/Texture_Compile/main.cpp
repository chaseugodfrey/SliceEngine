#ifndef MAIN
#define MAIN

#include <iostream>
#include "TextureLoader.h"

int main(int argc, char** argv)
{
	std::cout << "Current directory: " << std::filesystem::current_path() << std::endl;
	std::filesystem::path desc_file{ argv[0] };
	if (desc_file.extension() != ".desc") {
		std::cerr << "missing descriptor file to compile argv:"<< argv[0] << std::endl;
		std::exit(EXIT_FAILURE);
	}

	Texture_Asset::Texture_Compiler compiler{};

	compiler.Compile(desc_file);
}	

#endif
