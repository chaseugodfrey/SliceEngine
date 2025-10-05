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
		void DeserializeScene(std::filesystem::path const& filePath);
		json SerializeGameObject(entt::entity entity, entt::registry& registry);


		//failed attempt at genericising serialization types :((((((
		//template<typename T>
		//void SerializeValue(json& output, std::string const& name, std::string const& propName, entt::sparse_set& storage, rttr::variant& PropVal)
		//{
		//	output[name][storage.type().name()][propName] = propVal.get_value<T>();
		//}

		//template<typename K, typename V>
		//void SerializeMap(const std::map<K, V>& m, json& output)
		//{
		//	for (const auto& [key, value] : m)
		//	{
		//		std::string keyStr = key_to_string(key);
		//		output[keyStr] = serialize_value(value);
		//	}
		//}

		//template<typename K, typename V>
		//std::map<K, V> DeserializeMap(const json& input)
		//{
		//	std::map<K, V> result;
		//	for (auto& [keyStr, val] : input.items())
		//	{
		//		K key = string_to_key<K>(keyStr);
		//		V value = deserialize_value<V>(val);
		//		result.emplace(key, value);
		//	}
		//	return result;
		//}

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