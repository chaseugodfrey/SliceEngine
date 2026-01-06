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

#include <rttr/variant.h>

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

#pragma region Prefab Serialization
		std::string SerializePrefab(entt::entity entity);
		void SerializePrefabChild(json& output, entt::entity entity, entt::registry& registry);
		Entity DeserializePrefab(std::filesystem::path const& filePath, bool Editor = false);
		std::unordered_map<unsigned int, std::vector<rttr::variant>> DeserializePrefabComponents(std::filesystem::path const& filePath);

#pragma endregion
		json SerializeSceneResources();
		void DeserializeSceneResource(std::filesystem::path const& filePath);

		nlohmann::json GetJsonFromVariant(rttr::variant v);
		nlohmann::json VariantToJson(rttr::variant v);

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

		// For JPH::Vec3
		template<>
		inline void Serialize<JPH::Vec3>(json& output, const std::string& name, const std::string_view& typeName,
			const std::string& propName, const JPH::Vec3& v, const Entity& entity)
		{
			output[name][typeName][propName] = { v.GetX(), v.GetY(), v.GetZ()};
		}

		// For ColliderShape::BoxData
		template<>
		inline void Serialize<ColliderShape::BoxData>(json& output, const std::string& name, const std::string_view& typeName,
			const std::string& propName, const ColliderShape::BoxData& data, const Entity& entity)
		{
			output[name][typeName][propName]["scale"] = { data.scale.GetX(), data.scale.GetY(), data.scale.GetZ() };
		}

		// For ColliderShape::SphereData
		template<>
		inline void Serialize<ColliderShape::SphereData>(json& output, const std::string& name, const std::string_view& typeName,
			const std::string& propName, const ColliderShape::SphereData& data, const Entity& entity)
		{
			output[name][typeName][propName]["radius"] = data.radius;
		}

		// For ColliderShape::CapsuleData
		template<>
		inline void Serialize<ColliderShape::CapsuleData>(json& output, const std::string& name, const std::string_view& typeName,
			const std::string& propName, const ColliderShape::CapsuleData& data, const Entity& entity)
		{
			output[name][typeName][propName]["radius"] = data.radius;
			output[name][typeName][propName]["height"] = data.height;
		}

		template<>
		inline void Serialize<RigidBody::FreezeOptions>(json& output, const std::string& name, const std::string_view& typeName,
			const std::string& propName, const RigidBody::FreezeOptions& data, const Entity& entity)
		{
			output[name][typeName][propName] = { data.freezeX, data.freezeY, data.freezeZ };
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

		// For ScriptableFieldMap
		template<>
		inline void Serialize<std::unordered_map<std::string, rttr::variant>>(json& output, const std::string& name, const std::string_view& typeName,
			const std::string& propName, const std::unordered_map<std::string, rttr::variant>& value, const Entity& entity)
		{
			for (const auto& [k, v] : value)
			{
				output[name][typeName][propName][k] = VariantToJson(v);
			}
		}


		// Handle
		template<typename T>
		inline void Serialize(json& output, const std::string& name, const std::string_view& typeName,
			const std::string& propName, const Handle<T>& value, const Entity& entity)
		{
			if (value.getGUID() == GUID::null())
				output[name][typeName][propName]["GUID"] = "";
			else
			{
				output[name][typeName][propName]["GUID"] = std::to_string(value.getGUID().GetGUID());
				Core::GetInstance()->GetResourceManager()->mGUIDToSerialize.insert(value.getGUID());
			}
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

			//if (propName == "mName" && componentName == typeid(SliceEntity).name())
			//{
			//	FactoryInstance.UpdateName(value, entity);
			//}
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

		// Handle
		template <typename T>
		inline void Deserialize(rttr::variant& componentInstance, rttr::property& prop,
			const Handle<T>& value, const std::string& propName, const std::string& componentName,
			const Entity& entity)
		{
			Handle<T> handle;
			handle.mGUID = value.getGUID();
			std::string msg = "GUID Being deserialized : " + std::to_string(value.getGUID().GetGUID());
			SLICE_LOG_DEBUG(msg);
			prop.set_value(componentInstance, handle);
		}

		template <typename T>
		bool TryDeserializeType(rttr::variant& componentInstance, rttr::property& prop, 
			const json& value, const std::string& propName, const std::string& componentName,
			const Entity& entity)
		{
			if (prop.get_type() == rttr::type::get<T>()) 
			{
				try
				{
					// Attempt to get JSON value as T
					T val = value.get<T>();
					Deserialize<T>(componentInstance, prop, val, propName, componentName, entity);
					return true;
				}
				catch (const nlohmann::json::exception& e)
				{
					// Fallback or error handling
					std::ostringstream oss;
					oss << "[TryDeserializeType] Failed to parse JSON for property '"
						<< propName << "' in component '" << componentName << "'.\n"
						<< "Expected type: " << rttr::type::get<T>().get_name().to_string() << "\n"
						<< "Error: " << e.what() << "\n"
						<< "JSON value: " << value.dump();

					SLICE_LOG_ERROR(oss.str());
				}
				return true;
			}			
			return false;
		}

		// Special handling for ColliderShape::BoxData
		template<>
		inline bool TryDeserializeType<ColliderShape::BoxData>(rttr::variant& componentInstance, rttr::property& prop,
			const json& value, const std::string& propName, const std::string& componentName, const Entity& entity)
		{
			if (prop.get_type() == rttr::type::get<ColliderShape::BoxData>()) {
				ColliderShape::BoxData data;
				if (value.contains("scale") && value["scale"].is_array() && value["scale"].size() == 3) {
					data.scale = JPH::Vec3(value["scale"][0], value["scale"][1], value["scale"][2]);
				}
				prop.set_value(componentInstance, data);
				return true;
			}
			return false;
		}

		// Similar for SphereData
		template<>
		inline bool TryDeserializeType<ColliderShape::SphereData>(rttr::variant& componentInstance, rttr::property& prop,
			const json& value, const std::string& propName, const std::string& componentName, const Entity& entity)
		{
			if (prop.get_type() == rttr::type::get<ColliderShape::SphereData>()) {
				ColliderShape::SphereData data;
				if (value.contains("radius")) {
					data.radius = value["radius"];
				}
				prop.set_value(componentInstance, data);
				return true;
			}
			return false;
		}

		// Similar for CapsuleData
		template<>
		inline bool TryDeserializeType<ColliderShape::CapsuleData>(rttr::variant& componentInstance, rttr::property& prop,
			const json& value, const std::string& propName, const std::string& componentName, const Entity& entity)
		{
			if (prop.get_type() == rttr::type::get<ColliderShape::CapsuleData>()) {
				ColliderShape::CapsuleData data;
				if (value.contains("radius")) {
					data.radius = value["radius"];
				}
				if (value.contains("height")) {
					data.height = value["height"];
				}
				prop.set_value(componentInstance, data);
				return true;
			}
			return false;
		}

		template<>
		inline bool TryDeserializeType<RigidBody::FreezeOptions>(rttr::variant& componentInstance, rttr::property& prop,
			const json& value, const std::string& propName, const std::string& componentName, const Entity& entity)
		{
			if (prop.get_type() == rttr::type::get<RigidBody::FreezeOptions>()) {
				RigidBody::FreezeOptions data;
				if (value.is_array() && value.size() == 3) {
					data.freezeX = value[0];
					data.freezeY = value[1];
					data.freezeZ = value[2];
				}
				prop.set_value(componentInstance, data);
				return true;
			}
			return false;
		}

		template <typename... Types>
		void DeserializeProp(rttr::variant& componentInstance, rttr::property& prop,
			const json& value, const std::string& propName, const std::string& componentName,
			const Entity& entity)
		{
			//if (propName == "scriptableFieldMap")
			//{
			//	return;
			//}
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

					try
					{
						std::string defaultStr = value.get<std::string>();
						SLICE_LOG_ERROR(oss.str());
						Deserialize<std::string>(componentInstance, prop, defaultStr, propName, componentName, entity);
					}
					catch (const nlohmann::json::exception& e)
					{
						SLICE_LOG_ERROR("[DeserializeProp] Fallback string conversion failed for "
							+ componentName + "::" + propName + " — " + std::string(e.what()));

					}														
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

	// Handle
	template <typename T>
	inline void from_json(const json& j, Handle<T>& handle)
	{
		// Check if the JSON is null or not an object with the "GUID" key
		if (j.is_null() || !j.is_object() || !j.contains("GUID"))
		{
			handle = Handle<T>{};
			return;
		}

		// Get the "GUID" key from the object, which is a string (or null).
		// Then, deserialize that string value into the handle's mGUID member.
		j.at("GUID").get_to(handle.mGUID);
		std::string msg = "Deserialized Handle with GUID: " + std::to_string(handle.mGUID.GetGUID());
		SLICE_LOG_DEBUG(msg);

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
namespace JPH
{
	inline void from_json(const json& j, JPH::Vec3& v) {
		v.SetX(j.at(0).get<float>());
		v.SetY(j.at(1).get<float>());
		v.SetZ(j.at(2).get<float>());
	}
	inline void to_json(json& j, const JPH::Vec3& v) {
		j = json::array({ v.GetX(), v.GetY(), v.GetZ() });
	}
}

namespace rttr
{
	inline rttr::variant JsonToVariant(const nlohmann::json& j)
	{
		if (j.is_object() && j.contains("Type") && j.contains("Value"))
		{
			const std::string& typeName = j["Type"].get<std::string>();
			const nlohmann::json& valueJson = j["Value"];

			// Primitive types
			if (typeName == "float") 
			{
				return rttr::variant(valueJson.get<float>());
			}
			if (typeName == "int")
			{
				return rttr::variant(valueJson.get<int>());
			}
			if (typeName == "bool") 
			{
				return rttr::variant(valueJson.get<bool>());
			}
			if (typeName == "std::string")
			{
				return rttr::variant(valueJson.get<std::string>());
			}
			if (typeName == "double") 
			{
				return rttr::variant(valueJson.get<double>());
			}

			// GLM types
			if (typeName == "glm::vec3") 
			{
				return rttr::variant(valueJson.get<glm::vec3>());
			}
			if (typeName == "glm::vec2")
			{
				return rttr::variant(valueJson.get<glm::vec2>());
			}

			// JPH types
			if (typeName == "JPH::Vec3")
			{
				return rttr::variant(valueJson.get<JPH::Vec3>());
			}

			// Vector types
			if (typeName == "std::vector<float>")
			{
				return rttr::variant(valueJson.get<std::vector<float>>());
			}
			if (typeName == "std::vector<glm::vec3>") 
			{
				return rttr::variant(valueJson.get<std::vector<glm::vec3>>());
			}
			if (typeName == "std::vector<std::string>") 
			{
				return rttr::variant(valueJson.get<std::vector<std::string>>());
			}
			if (typeName == "std::vector<int>")
			{
				return rttr::variant(valueJson.get<std::vector<int>>());
			}

			return rttr::variant(valueJson.get<std::string>());
		}
		else
		{
			SLICE_LOG_ERROR("JsonToVariant doesnt match any supported type.");
			return rttr::variant();
		}
	}

	/// <summary>
	/// Mostly used for script component as well since script fields are stored as variants
	/// converts json into a variatn value
	/// </summary>
	/// <param name="j"></param>
	/// <param name="um"></param>
	inline void from_json(const json& j, std::unordered_map<std::string, rttr::variant>& um)
	{
		um.clear();
		if (!j.is_object()) return;

		for (auto it = j.begin(); it != j.end(); ++it)
		{
			const std::string& key = it.key();
			const json& val = it.value();

			um[key] = JsonToVariant(val);
		}
	}
}

#pragma endregion

#endif