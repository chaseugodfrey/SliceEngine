#include <iostream>
#include "TextureLoader.h"
#include <json.hpp>
#include <fstream>

int main(int argc, char** argv)
{
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


	if (!obj.contains("assetPath") || !obj.contains("resourcePath")) {
		std::cout << "Invalid metafile" << std::endl;
		return  -1;
	}

	std::string asset_path = obj["assetPath"];
	std::string resource_path = obj["resourcePath"];

	Texture_Asset::Texture_Compiler compiler{};

	compiler.Compile_Asset(asset_path.c_str(), resource_path.c_str());
}	
