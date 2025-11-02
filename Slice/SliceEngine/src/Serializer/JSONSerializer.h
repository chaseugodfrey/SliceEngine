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
		std::unordered_map<uint32_t, uint32_t> DeserializeScene(std::filesystem::path const& filePath);
		json SerializeGameObject(entt::entity entity, entt::registry& registry);

		std::string SerializePrefab(entt::entity entity);
		void SerializePrefabChild(json& output, entt::entity entity, entt::registry& registry);

		Entity DeserializePrefab(std::filesystem::path const& filePath);

		// Add more templates in this region should the current templates do not serve your data type well
#pragma region Serialization Templates
		// For generic values
		template <typename T>
		void Serialize(json& output, const std::string& name, const std::string_view& typeName,
			const std::string& propName, const T& value, const Entity& entity)
		{
			output[name][typeName][propName] = value;
		}

		//For GUID
		template <>
		inline void Serialize<GUID>(json& output, const std::string& name, const std::string_view& typeName,
			const std::string& propName, const GUID& value, const Entity& entity)
		{
			if (value == GUID::null())
				output[name][typeName][propName] = "";
			else
			{
				output[name][typeName][propName] = std::to_string(value.GetGUID());
				Core::GetInstance()->GetResourceManager()->mGUIDToSerialize.insert(value);
			}


		}

		// For generic vectors
		template <typename T>
		void Serialize(json& output, const std::string& name,
			const std::string_view& typeName, const std::string& propName,
			const std::vector<T>& vec, const Entity& entity)
		{
			auto& arr = output[name][typeName][propName];
			arr = json::array();

			for (const auto& val : vec)
			{
				json elem;
				Serialize(elem, name, typeName, propName, val, entity);

				// extract the inner serialized value if wrapped
				if (elem.contains(name) && elem[name].contains(typeName) && elem[name][typeName].contains(propName))
					arr.push_back(elem[name][typeName][propName]);
				else
					arr.push_back(val);
			}
		}

		// For generic arrays
		template <typename T, size_t N>
		void Serialize(json& output, const std::string& name,
			const std::string_view& typeName, const std::string& propName,
			const std::array<T, N>& arr, const Entity& entity)
		{
			auto& jArr = output[name][typeName][propName];
			jArr = json::array();

			for (const auto& val : arr)
			{
				json elem;
				Serialize(elem, name, typeName, propName, val, entity);

				if (elem.contains(name) && elem[name].contains(typeName) && elem[name][typeName].contains(propName))
					jArr.push_back(elem[name][typeName][propName]);
				else
					jArr.push_back(val);
			}
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

		// For glm::quat
		template<>
		inline void Serialize<glm::quat>(json& output, const std::string& name, const std::string_view& typeName,
			const std::string& propName, const glm::quat& q, const Entity& entity)
		{
			output[name][typeName][propName] = { q.w, q.x, q.y, q.z };
		}

		// For unsigned char
		template <>
		inline void Serialize<unsigned char>(json& output, const std::string& name, const std::string_view& typeName,
			const std::string& propName, const unsigned char& value, const Entity& entity)
		{
			output[name][typeName][propName] = std::to_string(static_cast<uint64_t>(value));
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
				rttr::type propType = propVal.get_type();
				if (propType.is_enumeration())
				{
					// this line looks kinda cancer ngl
					output[name][typeName][propName] = propType.get_enumeration().value_to_name(propVal).to_string();
				}
				else
				{
					// fallback: convert to string
					output[name][typeName][propName] = propVal.to_string();
				}
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

		// For generic vectors
		template <typename T>
		void Deserialize(rttr::variant& componentInstance, rttr::property& prop,
			const std::vector<T>& vec, const std::string& propName, const std::string& componentName,
			const Entity& entity)
		{
			std::vector<T> result;
			result.reserve(vec.size());

			for (size_t i = 0; i < vec.size(); ++i)
			{
				const T& elem = vec[i];

				if constexpr (std::is_same_v<T, std::vector<typename T::value_type>>)
				{
					// Nested vector — recurse
					std::vector<typename T::value_type> innerResult;
					Deserialize(componentInstance, prop, elem, propName, componentName, entity);
					result.push_back(elem);
				}
				else
				{
					// Base case — just add the element
					result.push_back(elem);
				}
			}

			prop.set_value(componentInstance, result);
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
				rttr::type propType = prop.get_type();
				// for enumerations
				if (propType.is_enumeration())
				{
					auto enumStr = value.get<std::string>();
					// this line looks a bit less cancer
					rttr::variant enumVal = propType.get_enumeration().name_to_value(enumStr);
					if (enumVal.is_valid())
					{
						prop.set_value(componentInstance, enumVal);
					
					}
				}
				else
				{
					std::ostringstream oss;
					oss << "[DeserializeProp] Unhandled property type during deserialization\n"
						<< " Component: " << componentInstance.get_type().get_name().to_string() << "\n"
						<< " Property:  " << prop.get_name().to_string() << "\n"
						<< " Expected Type: " << prop.get_type().get_name().to_string() << "\n"
						<< " JSON Value: " << value.dump() << "\n"
						<< " Falling back to string deserialization.";

					SLICE_LOG_ERROR(oss.str());
					Deserialize<std::string>(componentInstance, prop, value, propName, componentName, entity);
				}

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
	inline void to_json(json& j, const glm::vec2& v) {
		j = json::array({ v.x, v.y });
	}

	inline void from_json(const json& j, glm::vec3& v) {
		v.x = j.at(0).get<float>();
		v.y = j.at(1).get<float>();
		v.z = j.at(2).get<float>();
	}
	inline void to_json(json& j, const glm::vec3& v) {
		j = json::array({ v.x, v.y, v.z });
	}

	inline void from_json(const json& j, glm::vec4& v) {
		v.x = j.at(0).get<float>();
		v.y = j.at(1).get<float>();
		v.z = j.at(2).get<float>();
		v.w = j.at(3).get<float>();
	}
	inline void to_json(json& j, const glm::vec4& v) {
		j = json::array({ v.x, v.y, v.z, v.w });
	}


	inline void from_json(const json& j, glm::quat& q)
	{
		q.w = j.at(0).get<float>();
		q.x = j.at(1).get<float>();
		q.y = j.at(2).get<float>();
		q.z = j.at(3).get<float>();
	}
	inline void to_json(json& j, const glm::quat& q)
	{
		j = json::array({ q.w, q.x, q.y, q.z });
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
			e = static_cast<entt::entity>(j.get<uint32_t>());
		}
	}

	inline void to_json(json& j, const entt::entity& e)
	{
		j = static_cast<uint32_t>(e);
	}
}
#pragma endregion

#endif