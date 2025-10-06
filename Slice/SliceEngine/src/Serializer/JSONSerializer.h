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
		void SerializeFile(json const& input, std::filesystem::path const& filePath);
		json SerializeGameObject(GameObject& node);
		void SerializeScene(std::filesystem::path const& filePath);
		json DeserializeFile(std::filesystem::path const& filePath);
		std::unordered_map<uint64_t, uint64_t> DeserializeScene(std::filesystem::path const& filePath);
		json SerializeGameObject(entt::entity entity, entt::registry& registry);

		template <typename T>
		void Serialize(json& output, const std::string& name, const std::string& typeName,
			const std::string& propName, const T& value)
		{
			output[name][typeName][propName] = value;
		}

		template <typename T>
		void Serialize(json& output, const std::string& name, const std::string& typeName,
			const std::string& propName, const std::vector<T>& vec)
		{
			for (size_t i = 0; i < vec.size(); ++i)
				Serialize(output, name, typeName, propName + "[" + std::to_string(i) + "]", vec[i]);
		}

		template <typename T, size_t N>
		void Serialize(json& output, const std::string& name,
			const std::string& typeName, const std::string& propName,
			const std::array<T, N>& value)
		{
			for (size_t i = 0; i < N; ++i)
				output[name][typeName][propName][i] = value[i];
		}
		template <>
		inline void Serialize<std::array<Entity, 4>>(json& output, const std::string& name,
			const std::string& typeName, const std::string& propName,
			const std::array<Entity, 4>& value)
		{
			for (size_t i = 0; i < 4; ++i)
			{
				Entity e = value[i];
				if (e == entt::null)
					output[name][typeName][propName][i] = nullptr;
				else
					output[name][typeName][propName][i] = static_cast<uint64_t>(e);
			}
		}

		inline void Serialize(json& output, const std::string& name, const std::string& typeName,
			const std::string& propName, const glm::vec2& v)
		{
			output[name][typeName][propName] = { v.x, v.y };
		}

		inline void Serialize(json& output, const std::string& name, const std::string& typeName,
			const std::string& propName, const glm::vec3& v)
		{
			output[name][typeName][propName] = { v.x, v.y, v.z };
		}

		inline void Serialize(json& output, const std::string& name, const std::string& typeName,
			const std::string& propName, const glm::vec4& v)
		{
			output[name][typeName][propName] = { v.x, v.y, v.z, v.w };
		}

		template <typename T>
		bool TrySerializeType(json& output, const std::string& name,
			const std::string& typeName, const std::string& propName,
			const rttr::variant& propVal)
		{
			//for cases that dont need the get value
			if (propVal.is_type<EntityID>())
			{
				Serialize(output, name, typeName, propName, propVal.get_value<T>());
				return true;
			}
			else if (propVal.is_type<T>())
			{
				Serialize(output, name, typeName, propName, propVal.get_value<T>());
				return true;
			}
			return false;
		}
		template <typename... Types>
		void SerializeProp(json& output, const std::string& name,
			const std::string_view& typeName, const std::string& propName,
			const rttr::variant& propVal)
		{
			// stop at first successful serialization
			bool handled = (TrySerializeType<Types>(output, name, typeName, propName, propVal) || ...);

			if (!handled)
			{
				// fallback: convert to string
				output[name][typeName][propName] = propVal.to_string();
			}
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