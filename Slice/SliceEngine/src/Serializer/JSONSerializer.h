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
	namespace JSONSerializer
	{
		void SerializeFile(json const& input, std::filesystem::path const& filePath);
		json SerializeGameObject(GameObject& node);
		void SerializeScene(std::filesystem::path const& filePath);
		json DeserializeFile(std::filesystem::path const& filePath);
		std::unordered_map<uint64_t, uint64_t> DeserializeScene(std::filesystem::path const& filePath);
		json SerializeGameObject(entt::entity entity, entt::registry& registry);

		// Add more templates in this region should the current templates do not serve your data type well
#pragma region SerializationTemplates
		// For generic values
		template <typename T>
		void Serialize(json& output, const std::string& name, const std::string_view& typeName,
			const std::string& propName, const T& value, const Entity& entity)
		{
			output[name][typeName][propName] = value;
		}

		// For generic vectors
		template <typename T>
		void Serialize(json& output, const std::string& name, const std::string_view& typeName,
			const std::string& propName, const std::vector<T>& vec, const Entity& entity)
		{
			output[name][typeName][propName] = json::array();
			for (size_t i = 0; i < vec.size(); ++i)
				Serialize(output, name, typeName, propName + "[" + std::to_string(i) + "]", vec[i],entity);
		}

		// For generic arrays
		template <typename T, size_t N>
		void Serialize(json& output, const std::string& name,
			const std::string_view& typeName, const std::string& propName,
			const std::array<T, N>& value, const Entity& entity)
		{
			for (size_t i = 0; i < N; ++i)
				output[name][typeName][propName][i] = value[i];
		}

		// For Relationship array (up down left right stuff)
		template <>
		inline void Serialize<std::array<Entity, 4>>(json& output, const std::string& name,
			const std::string_view& typeName, const std::string& propName,
			const std::array<Entity, 4>& value, const Entity& entity)
		{
			for (size_t i = 0; i < 4; ++i)
			{
				Entity e = value[i];
				if (e == entt::null || e == std::numeric_limits<entt::entity>::max())
					output[name][typeName][propName][i] = nullptr;
				else
					output[name][typeName][propName][i] = static_cast<uint64_t>(e);
			}
		}

		// For Entity (entt::entity)
		template<>
		inline void Serialize(json& output, const std::string& name, const std::string_view& typeName,
			const std::string& propName, const EntityID& value, const Entity& entity)
		{
			output[name][typeName][propName] = entity;
		}

		// For glm::vec2
		template<>
		inline void Serialize<glm::vec2>(json& output, const std::string& name, const std::string_view& typeName,
			const std::string& propName, const glm::vec2& v, const Entity& entity)
		{
			output[name][typeName][propName] = { v.x, v.y };
		}

		// For glm::vec3
		template<>
		inline void Serialize<glm::vec3>(json& output, const std::string& name, const std::string_view& typeName,
			const std::string& propName, const glm::vec3& v, const Entity& entity)
		{
			output[name][typeName][propName] = { v.x, v.y, v.z };
		}

		// For glm::vec4
		template<>
		inline void Serialize<glm::vec4>(json& output, const std::string& name, const std::string_view& typeName,
			const std::string& propName, const glm::vec4& v, const Entity& entity)
		{
			output[name][typeName][propName] = { v.x, v.y, v.z, v.w };
		}

		// Main evaluater for serialization
		template <typename T>
		bool TrySerializeType(json& output, const std::string& name,
			const std::string_view& typeName, const std::string& propName,
			const rttr::variant& propVal, const Entity& entity)
		{
			if (propVal.is_type<T>())
			{
				Serialize(output, name, typeName, propName, propVal.get_value<T>(), entity);
				return true;
			}
			return false;
		}

		// Main "Looper" of all types declared to be supported
		template <typename... Types>
		void SerializeProp(json& output, const std::string& name,
			const std::string_view& typeName, const std::string& propName,
			const rttr::variant& propVal, const Entity& entity)
		{
			// stop at first successful serialization
			bool handled = (TrySerializeType<Types>(output, name, typeName, propName, propVal, entity) || ...);

			if (!handled)
			{
				// fallback: convert to string
				output[name][typeName][propName] = propVal.to_string();
			}
		}
#pragma endregion

		// Add more templates in this region should the current templates do not serve your data type well
#pragma region Deserialization Templates
		// For generic values
		template <typename T>
		void Deserialize(rttr::variant& componentInstance, rttr::property& prop, 
			const T& value)
		{
			prop.set_value(componentInstance, value);
		}

		// For Relationship array (up down left right stuff)
		template <>
		inline void Deserialize<std::array<Entity, 4>>(rttr::variant& componentInstance, rttr::property& prop,
			const std::array<Entity,4>& value)
		{
			std::array<Entity, 4> arr;
			for (size_t i = 0; i < arr.size(); ++i)
			{
				auto v = value[i];

				if (v == std::numeric_limits<entt::entity>::max())
				{
					arr[i] = entt::null;
				}
				else
				{
					arr[i] = v;
				}
			}
			prop.set_value(componentInstance, arr);
		}
		// For Entity (entt::entity)
		template <>
		inline void Deserialize<EntityID>(rttr::variant& componentInstance, rttr::property& prop, const EntityID& value)
		{
			uint64_t rawID = value.value;
			prop.set_value(componentInstance, EntityID{ rawID });
		}
		template <typename T>
		bool TryDeserializeType(rttr::variant& componentInstance,
			rttr::property& prop,
			const json& value)
		{
			if (prop.get_type() == rttr::type::get<T>()) {
				Deserialize<T>(componentInstance, prop, value.get<T>());
				return true;
			}
			return false;
		}
		template <typename... Types>
		void DeserializeProp(rttr::variant& componentInstance,
			rttr::property& prop,
			const json& value)
		{
			bool handled = (TryDeserializeType<Types>(componentInstance, prop, value) || ...);

			if (!handled)
			{
				SLICE_LOG_ERROR(value.dump() + " is not handled in deserialization process.");
			}
		}


#pragma endregion
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

	inline void from_json(const nlohmann::json& j, EntityID& e) {
		e.value = j.get<uint64_t>();  // assuming your JSON stores it as a number
	}

	inline void from_json(const nlohmann::json& j, glm::vec2& v) {
		v.x = j.at(0).get<float>();
		v.y = j.at(1).get<float>();
	}

	inline void from_json(const nlohmann::json& j, glm::vec3& v) {
		v.x = j.at(0).get<float>();
		v.y = j.at(1).get<float>();
		v.z = j.at(2).get<float>();
	}

	inline void from_json(const nlohmann::json& j, glm::vec4& v) {
		v.x = j.at(0).get<float>();
		v.y = j.at(1).get<float>();
		v.z = j.at(2).get<float>();
		v.w = j.at(3).get<float>();
	}
}


#endif