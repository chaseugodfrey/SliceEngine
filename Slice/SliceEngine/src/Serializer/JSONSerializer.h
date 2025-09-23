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
		json SerializeGameObject(GameObject& node);
		json Deserialize(std::filesystem::path const& filePath);
		std::vector<GameObject> DeserializeGameObjects(json const& input);

		namespace Tests
		{
			//Take note of any errors and logs that can appear on the console during the tests
			//param cleanOutput = false to keep logs to assist in debugging. By default its true if console error logs are enough
			void RunTests(bool cleanOutput = true);
		}
	}	
}


#endif