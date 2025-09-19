#ifndef JSONSERIALIZER_H
#define JSONSERIALIZER_H

#include "Logger/Logger.h"
#include "Core/Core.h"
#include "json.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>

using json = nlohmann::json;

namespace SliceEngine
{
	//The workflow in my head
	//GameObject root = ...; // your scene root
	//json j = RecursiveSerialize(root);

	//// If you want to edit / validate / add metadata, do it here
	//j["metadata"] = "example";

	//Serialize(j, "scene.json");  // entomb it into a file

	namespace JSONSerializer
	{
		void Serialize(json const& input, std::filesystem::path const& filePath);
		//GameObject in place of scenegraph node as placeholder
		json RecursiveSerialize(GameObject& node);
		json Deserialize(std::filesystem::path const& filePath);
		//GameObject in place of scenegraph node as placeholder
		GameObject RecursiveDeserialize(json const& input);

		//Creates a file called TestSerialize.scene to test serialization and deserialization with end result being in Testing/Temporary/TestSerialize.scene
		void Test();

		void Test2();
	}	
}


#endif