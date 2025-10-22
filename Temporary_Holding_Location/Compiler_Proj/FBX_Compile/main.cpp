#include "Assimp_Importer.h"

#define NO_INPUT 0

#if COMPILE_ONLY
#include <json.hpp>
#else
#include "OpenGL_Starter.h"	//this is only for testing
#endif
int main(int argc, char** argv)
{
#if COMPILE_ONLY	//only load compile and save

#if !NO_INPUT
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
#else
	//kinda dosent work rn since no json
	std::string asset_path = "../Asset/player.fbx";
	std::string resource_path = "../Asset/player.mdl";
#endif
	//Read the descriptor file here i guess?

	Geometry::FBX_Compiler compiler{};

	compiler.Compile_Asset(asset_path.c_str(), resource_path.c_str(), obj);
	//compiler.Compile_Primitive(Geometry::Mesh_Compiler::Cube, resource_path2.c_str());
	return 1;

#else	//load, compile, save, draw
	OpenGL_Starter proj;
	proj.Init();
	proj.Update();
	proj.Terminate();
#endif
}