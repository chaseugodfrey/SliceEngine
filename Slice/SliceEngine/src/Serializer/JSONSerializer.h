/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			JSONSerializer.h
 author:		Hafiz
 email:			b.muhammadhafiz@digipen.edu
 brief:			Serialize and Deserialize JSON data

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
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
		void SerializeScene(std::filesystem::path const& filePath);
		json Deserialize(std::filesystem::path const& filePath);
		std::unordered_map<uint64_t, uint64_t> DeserializeScene(std::filesystem::path const& filePath);
		json SerializeGameObject(entt::entity entity, entt::registry& registry);


		//genericising serialization types of maps?????? if SerializeValue is of container type, might explode		
		template<typename t>
		void SerializeValue(json& output, std::string const& name, std::string const& propname, entt::sparse_set& storage, rttr::variant& propval)
		{
			output[name][storage.type().name()][propname] = propval.get_value<t>();
		}

		template<typename k, typename v>
		void SerializeMap(const std::unordered_map<k, v>& m, json& output)
		{
			for (const auto& [key, value] : m)
			{
				std::string keystr = key_to_string(key);
				output[keystr] = serialize_value(value);
			}
		}

		template<typename k, typename v>
		std::unordered_map<k, v> DeserializeMap(const json& input)
		{
			std::map<k, v> result;
			for (auto& [keystr, val] : input.items())
			{
				k key = string_to_key<k>(keystr);
				v value = deserialize_value<v>(val);
				result.emplace(key, value);
			}
			return result;
		}

		namespace Tests
		{
			enum TestNum
			{
				TEST1 = 1,
				TEST2 = 2,
				TEST3 = 3,
				TEST4 = 4,
				ALLTESTS = 0
			};

			//Take note of any errors and logs that can appear on the console during the tests
			//param cleanOutput = false to keep logs to assist in debugging. By default its true if console error logs are enough
			void RunTests(TestNum testNum = ALLTESTS, bool cleanOutput = true);
		}
	}	
}


#endif