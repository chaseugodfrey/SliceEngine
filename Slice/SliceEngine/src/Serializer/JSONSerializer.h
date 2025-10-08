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
#pragma region Serialization Templates
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
				Serialize(output, name, typeName, propName + "[" + std::to_string(i) + "]", vec[i], entity);
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

		// For unsigned char
		template <>
		inline void Serialize<unsigned char>(json& output, const std::string& name, const std::string_view& typeName,
			const std::string& propName, const unsigned char& value, const Entity& entity)
		{
			output[name][typeName][propName] = static_cast<uint64_t>(value);
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
		// For deserialization, may need to add more from_json functions in the same namespace as the variable to be getted from
#pragma region Deserialization Templates
		// For generic values
		template <typename T>
		void Deserialize(rttr::variant& componentInstance, rttr::property& prop,
			const T& value, const std::string& propName, const std::string& componentName,
			const Entity& entity)
		{
			prop.set_value(componentInstance, value);
		}

		// For generic strings + special exceptions
		template <>
		inline void Deserialize<std::string>(rttr::variant& componentInstance, rttr::property& prop,
			const std::string& value, const std::string& propName, const std::string& componentName,
			const Entity& entity)
		{
			prop.set_value(componentInstance, value);

			if (propName == "mName" && componentName == typeid(SliceEntity).name())
			{
				FactoryInstance.UpdateName(value, entity);
			}
		}

		// For Relationship array (up down left right stuff)
		template <>
		inline void Deserialize<std::array<Entity, 4>>(rttr::variant& componentInstance, rttr::property& prop,
			const std::array<Entity, 4>& value, const std::string& propName, const std::string& componentName,
			const Entity& entity)
		{
			std::array<Entity, 4> arr;
			for (size_t i = 0; i < arr.size(); ++i)
			{
				auto v = value[i];

				if (v == entt::null)
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
		template <typename T>
		bool TryDeserializeType(rttr::variant& componentInstance, rttr::property& prop, 
			const json& value, const std::string& propName, const std::string& componentName,
			const Entity& entity)
		{
			if (prop.get_type() == rttr::type::get<T>()) {
				Deserialize<T>(componentInstance, prop, value.get<T>(),propName, componentName, entity);
				return true;
			}
			return false;
		}
		template <typename... Types>
		void DeserializeProp(rttr::variant& componentInstance, rttr::property& prop,
			const json& value, const std::string& propName, const std::string& componentName,
			const Entity& entity)
		{
			bool handled = (TryDeserializeType<Types>(componentInstance, prop, value, propName, componentName, entity) || ...);

			if (!handled)
			{
				std::ostringstream oss;
				oss << "[DeserializeProp] Unhandled property type during deserialization\n"
					<< " Component: " << componentInstance.get_type().get_name().to_string() << "\n"
					<< " Property:  " << prop.get_name().to_string() << "\n"
					<< " Expected Type: " << prop.get_type().get_name().to_string() << "\n"
					<< " JSON Value: " << value.dump() << "\n"
					<< "Fallback to string deserialization.";

				SLICE_LOG_ERROR(oss.str());
				Deserialize<std::string>(componentInstance, prop, value, propName, componentName, entity);
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
}

// If .get<T> giving errors, add more support for your containers here
#pragma region json.hpp .get<T> Additional Type Support
namespace nlohmann 
{
	inline void from_json(const json& j, unsigned char& value)
	{
		if (j.is_number_unsigned())
		{
			// Direct numeric value
			uint64_t temp = j.get<uint64_t>();
			if (temp > 255)
				throw std::runtime_error("JSON value out of range for unsigned char: " + std::to_string(temp));
			value = static_cast<unsigned char>(temp);
		}
		else if (j.is_string())
		{
			std::string s = j.get<std::string>();
			if (s.empty())
			{
				value = 0; // Treat empty string as zero
			}
			else
			{
				uint64_t temp = std::stoull(s);
				if (temp > 255)
					throw std::runtime_error("JSON string value out of range for unsigned char: " + s);
				value = static_cast<unsigned char>(temp);
			}
		}
		else if (j.is_null())
		{
			value = 0; // Treat null as zero
		}
		else
		{
			throw std::runtime_error("Invalid JSON type for unsigned char: " + j.dump());
		}
	}
}

namespace SliceEngine
{
	// Deserialize GUID
	inline void from_json(const json& j, GUID& guid)
	{
		if (j.is_string())
		{
			std::string s = j.get<std::string>();
			if (s.empty())
			{
				guid = GUID::null();
			}
			else
			{
				guid = GUID(static_cast<uint64_t>(std::stoull(s)));
			}
		}
	}

	// Serialize GUID
	inline void to_json(json& j, const GUID& guid)
	{
		j = guid.GetGUID();
	}
}

namespace glm
{
	inline void from_json(const json& j, glm::vec2& v) {
		v.x = j.at(0).get<float>();
		v.y = j.at(1).get<float>();
	}

	inline void from_json(const json& j, glm::vec3& v) {
		v.x = j.at(0).get<float>();
		v.y = j.at(1).get<float>();
		v.z = j.at(2).get<float>();
	}

	inline void from_json(const json& j, glm::vec4& v) {
		v.x = j.at(0).get<float>();
		v.y = j.at(1).get<float>();
		v.z = j.at(2).get<float>();
		v.w = j.at(3).get<float>();
	}
}

namespace entt 
{
	inline void from_json(const json& j, entt::entity& e)
	{
		if (j.is_null()) {
			e = entt::null;
		}
		else {
			e = static_cast<entt::entity>(j.get<uint64_t>());
		}
	}

	inline void to_json(json& j, const entt::entity& e)
	{
		j = static_cast<uint64_t>(e);
	}
}
#pragma endregion

#endif