/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			JSONSerializer.cpp
 author:		Hafiz
 email:			b.muhammadhafiz@digipen.edu
 brief:			Serialize and Deserialize JSON data

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "JSONSerializer.h"

namespace SliceEngine
{
	namespace JSONSerializer
	{

		constexpr auto testPath("Assets/Tests/");

		bool AddComponentFromVariant(GameObject& node, rttr::variant const& componentInstance, std::string const& componentName)
		{
			rttr::type t = rttr::type::get_by_name(componentName);
			if (!t)
				return false;

			FactoryInstance.EmplaceComponents(node.GetEntity(), componentInstance);
		
			return false; // unknown type
		}

		//Save json (the data structure) to JSON (the file) Note: This replaces the file in the filepath with data in the given json
		void SerializeFile(json const& input, std::filesystem::path const& filePath)
		{
			std::ofstream ofs(filePath);
			if (!ofs.is_open())
			{
				Logger::LogError("JSONSerializer::Serialize", "Unable to write JSON to path: " + filePath.string());
				return;
			}
			ofs << input.dump(4);
			ofs.close();
		}

		void SerializeScene(std::filesystem::path const& filePath)
		{
			json output;

			auto& registry = Core::GetInstance()->GetRegistry();

			auto entityView = registry.view<SliceEntity>();
			for (auto entity : entityView)
			{
				output += SerializeGameObject(entity, registry);
			}

			SerializeFile(output, filePath);
		}

		json SerializeGameObject(entt::entity entity, entt::registry& registry)
		{
			json output;

			// Go through every registered component
			for (auto&& [type_id, storage] : registry.storage())
			{
				if (!storage.contains(entity))
				{
					continue; // entity does not have this component
				}

				std::string componentName(storage.type().name());

				rttr::type componentType = rttr::type::get_by_name(componentName);
				if (!componentType)
				{
					SLICE_LOG_ERROR(std::string(storage.type().name()) + " is not registered");
					continue;
				}

				auto it = Core::GetInstance()->mFactory.mComponentGetters.find(type_id);
				if (it == Core::GetInstance()->mFactory.mComponentGetters.end())
				{
					SLICE_LOG_ERROR(std::string(storage.type().name()) + " does not have a getter");
					// assert?

					continue;
				}

				rttr::variant componentData = it->second(registry, entity);

				for (const auto& property : componentType.get_properties())
				{
					// this should be the component's property data
					std::string propName = property.get_name().to_string();

					rttr::variant propVal = property.get_value(componentData);

					std::string name = FactoryInstance.GetGOByEntity(entity).GetName();

					if (!propVal.is_valid())
					{
						continue;
					}


					// To make it easy to see and add what types are supported. If added
					// but the output is wrong, might need to create a specialized variant
					// of Serialize(...) in the header file
					// Add supported types here + DeserializeProp below
					SerializeProp
					<
						int, 
						unsigned int,
						unsigned char,
						float, 
						double, 
						bool, 
						uint32_t,
						uint64_t,
						GUID,
						std::array<uint64_t, 4>, 
						std::array<Entity, 4>,
						std::vector<uint64_t>,
						glm::vec2, 
						glm::vec3, 
						glm::vec4,
						glm::quat,
						std::string
					>
						(output, name, storage.type().name(), propName, propVal, static_cast<Entity>(entity));

#pragma region Old Serialization Backup
					//if (propVal.is_type<int>())
					//{
					//	output[name][storage.type().name()][propName] = propVal.get_value<int>();
					//}
					//else if (propVal.is_type<unsigned int>())
					//{
					//	output[name][storage.type().name()][propName] = propVal.get_value<unsigned int>();
					//}
					//else if (propVal.is_type<float>())
					//{
					//	output[name][storage.type().name()][propName] = propVal.get_value<float>();
					//}
					//else if (propVal.is_type<double>())
					//{
					//	output[name][storage.type().name()][propName] = propVal.get_value<double>();
					//}
					//else if (propVal.is_type<bool>())
					//{
					//	output[name][storage.type().name()][propName] = propVal.get_value<bool>();
					//}
					//else if (propVal.is_type<uint64_t>())
					//{
					//	output[name][storage.type().name()][propName] = propVal.get_value<uint64_t>();
					//}
					//else if (propVal.is_type<EntityID>())
					//{
					//	//EntityID eid();//propVal.get_value<EntityID>();
					//	output[name][storage.type().name()][propName] = entity;
					//}
					//else if (propVal.is_type<std::array<uint64_t, 4>>())
					//{
					//	const auto& vec = propVal.get_value<std::array<uint64_t, 4>>();
					//	for (size_t i{}; i < 4; ++i)
					//	{
					//		output[name][storage.type().name()][propName][i] = vec[i];
					//	}
					//}
					//else if (propVal.is_type<std::array<Entity, 4>>())
					//{
					//	const auto& vec = propVal.get_value<std::array<Entity, 4>>();
					//	for (size_t i{}; i < 4; ++i)
					//	{
					//		Entity e = vec[i];
					//		if (e == entt::null)
					//		{
					//			output[name][storage.type().name()][propName][i] = nullptr;
					//		}
					//		else
					//		{
					//			output[name][storage.type().name()][propName][i] = static_cast<uint64_t>(e);
					//		}
					//	}
					//}
					//else if (propVal.is_type<std::string>())
					//{
					//	output[name][storage.type().name()][propName] = propVal.get_value<std::string>();
					//}
					//else if (propVal.is_type<std::vector<uint64_t>>())
					//{
					//	const auto& vec = propVal.get_value<std::vector<uint64_t>>();
					//	for (const auto& elem : vec)
					//	{
					//		output[name][storage.type().name()][propName].push_back(elem);
					//	}
					//}
					//else if (propVal.is_type<glm::vec2>())
					//{
					//	auto v = propVal.get_value<glm::vec2>();
					//	output[name][storage.type().name()][propName] = { v.x, v.y};
					//}
					//else if (propVal.is_type<glm::vec3>())
					//{
					//	auto v = propVal.get_value<glm::vec3>();
					//	output[name][storage.type().name()][propName] = { v.x, v.y, v.z };
					//}
					//else if (propVal.is_type<glm::vec4>())
					//{
					//	auto v = propVal.get_value<glm::vec4>();
					//	output[name][storage.type().name()][propName] = { v.x, v.y, v.z, v.w };
					//}
					//else
					//{
					//	// fallback
					//	output[name][storage.type().name()][propName] = propVal.to_string();
					//}
#pragma endregion

				}
			}

			return output;
		}

		json SerializeGameObject(GameObject& node)
		{
			auto& registry = Core::GetInstance()->GetRegistry();
			entt::entity entity = node.GetEntity();

			return SerializeGameObject(entity, registry);
		}

		//Loads JSON (the file) and returns it as a json (the data structure) that can be accessed and edited
		json DeserializeFile(std::filesystem::path const& filePath)
		{
			std::ifstream ifs(filePath);
			if (!ifs.is_open())
			{
				Logger::LogError("JSONSerializer::Deserialize", "Unable to find/load JSON in path: " + filePath.string());
				return json{};
			}
			
			json output;
			ifs >> output;
			return output;
		}

		std::unordered_map<uint32_t, uint32_t> DeserializeScene(std::filesystem::path const& filePath)
		{
			
			std::unordered_map<uint32_t, uint32_t> sceneGraphMap{};

			json input = DeserializeFile(filePath);
			for (auto& [name, components] : input.items())
			{
				auto& factory = Core::GetInstance()->mFactory;
				GameObject node = factory.CreateBlank();

				for (auto& [objName, objProps] : components.items())
				{
					for (auto& [componentName, props] : objProps.items())
					{
						rttr::type compType = rttr::type::get_by_name(componentName);
						if (!compType)
						{
							//SLICE_LOG_ERROR(componentName + " not registered");
							continue;
						}

						rttr::variant componentInstance = compType.create();
						if (!componentInstance.is_valid())
						{
							SLICE_LOG_ERROR("Failed to create instance of " + componentName);
							continue;
						}

						// Note for hafiz and me: I moved the mName to sliceentity component
						// technically there should never be an instance of two objects with the same name serialized
						// since factory checks for that
						// So i shouldn't have to check for duplicate names when deserializing
						// but keep a note incase it dies next time

						for (auto& [propName, value] : props.items())
						{
							rttr::property prop = compType.get_property(propName);							

							if (!prop.is_valid())
								continue;

							// First Pass
							// Add supported types here
							DeserializeProp
								<
								int,
								unsigned int,
								unsigned char,
								float,
								double,
								bool,
								uint64_t,
								GUID,
								std::array<uint64_t, 4>,
								std::array<Entity, 4>,
								std::vector<uint64_t>,
								glm::vec2,
								glm::vec3,
								glm::vec4,
								glm::quat,
								std::string
								>
								(componentInstance, prop, value, propName, componentName, node.GetEntity());

							// Anything that needs a second pass
							// scene graph map stuff
							if (propName == "entity_id" && componentName == typeid(SceneGraph).name())
							{
								uint32_t oldID = value.get<uint32_t>();
								sceneGraphMap[oldID] = entt::to_integral(node.GetEntity());
							}

#pragma region Old Deserialization Backup
							//if (prop.get_type() == rttr::type::get<int>())
							//	prop.set_value(componentInstance, value.get<int>());
							//else if (prop.get_type() == rttr::type::get<unsigned int>())
							//	prop.set_value(componentInstance, value.get<unsigned int>());
							//else if (prop.get_type() == rttr::type::get<float>())
							//	prop.set_value(componentInstance, value.get<float>());
							//else if (prop.get_type() == rttr::type::get<double>())
							//	prop.set_value(componentInstance, value.get<double>());
							//else if (prop.get_type() == rttr::type::get<bool>())
							//	prop.set_value(componentInstance, value.get<bool>());
							//else if (prop.get_type() == rttr::type::get<uint64_t>())
							//	prop.set_value(componentInstance, value.get<uint64_t>());
							//else if (prop.get_type() == rttr::type::get<std::string>())
							//	prop.set_value(componentInstance, value.get<std::string>());
							//else if (prop.get_type() == rttr::type::get<std::vector<uint64_t>>())
							//{
							//	std::vector<uint64_t> vec;
							//	for (auto& v : value)
							//	{
							//		vec.push_back(v.get<uint64_t>());
							//	}
							//	prop.set_value(componentInstance, vec);
							//}
							//else if (prop.get_type() == rttr::type::get<EntityID>())
							//{
							//	uint64_t rawID = value.get<uint64_t>();								
							//	prop.set_value(componentInstance, EntityID{ rawID });
							//	sceneGraphMap[rawID] = entt::to_integral(node.GetEntity());
							//}
							//else if (prop.get_type() == rttr::type::get<std::array<uint64_t, 4>>())
							//{
							//	std::array<uint64_t, 4> arr;
							//	arr = value;
							//	prop.set_value(componentInstance, arr);
							//}
							//else if (prop.get_type() == rttr::type::get<std::array<Entity, 4>>())
							//{
							//	std::array<Entity, 4> arr;
							//	for (size_t i = 0; i < arr.size(); ++i)
							//	{
							//		auto v = value[i];

							//		if (v.is_null())
							//		{
							//			arr[i] = entt::null;
							//		}
							//		else
							//		{
							//			arr[i] = static_cast<Entity>(v.get<uint64_t>());
							//		}
							//	}
							//	prop.set_value(componentInstance, arr);
							//}
							//else if (prop.get_type() == rttr::type::get<glm::vec2>())
							//{
							//	glm::vec2 vec{ value[0].get<float>(), value[1].get<float>() };
							//	prop.set_value(componentInstance, vec);
							//}
							//else if (prop.get_type() == rttr::type::get<glm::vec3>())
							//{
							//	glm::vec3 vec{ value[0].get<float>(), value[1].get<float>(), value[2].get<float>() };
							//	prop.set_value(componentInstance, vec);
							//}
							//else if (prop.get_type() == rttr::type::get<glm::vec4>())
							//{
							//	glm::vec4 vec{ value[0].get<float>(), value[1].get<float>(), value[2].get<float>(), value[3].get<float>() };
							//	prop.set_value(componentInstance, vec);
							//}
							////else if (prop.get_type() == rttr::type::get<std::unordered_map<int, int>>())
							////{
							////	
							////}

							//else 
							//{
							//	// fallback: try string
							//	prop.set_value(componentInstance, value.get<std::string>());
							//}
#pragma endregion
							
						}

						AddComponentFromVariant(node, componentInstance, componentName);
					}

				}
			}

			// Remapping Entity IDs after all GOs have been deserialized
			auto& registry = Core::GetInstance()->GetRegistry();
			auto& factory = Core::GetInstance()->mFactory;
			auto entityView = registry.view<SliceEntity>();
			for (auto entity : entityView)
			{
				if (!registry.any_of<SceneGraph>(entity))
				{
					continue;
				}

				auto& sceneGraphComponent = registry.get<SceneGraph>(entity);
				
				for (int i = 0; i < sceneGraphComponent.neighbours.size(); ++i)
				{
					auto it = sceneGraphMap.find((uint64_t)sceneGraphComponent.neighbours[i]);
					if (it != sceneGraphMap.end())
					{
						sceneGraphComponent.neighbours[i] = (Entity)it->second;
					}

					// if this is the new child of the root entity
					// for it to be the new child, up is the root and there is no left children
					if (sceneGraphComponent.neighbours[SceneGraph::UP] == factory.GetRootEntity() && sceneGraphComponent.neighbours[SceneGraph::LEFT] == entt::null)
					{
						auto& rootSceneGraph = registry.get<SceneGraph>(factory.GetRootEntity());
					}
				}
			}
			
			return sceneGraphMap;
		}


		namespace Tests
		{
			void Test1(bool cleanOutput)
			{
				SLICE_LOG("Test 1 Beginning...");
				json input;

				std::string root("Scene");
				int num_objects_in_test_scene{ 10 };

				//add a number stored as double
				input["Examples"]["pi"] = 3.141;

				//add a Boolean stored as bool
				input["Examples"]["happy"] = true;

				//add a string stored as std::string
				input["Examples"]["name"] = "Niels";

				//add another null object by passing nullptr
				input["Examples"]["nothing"] = nullptr;

				//add an object inside the object
				input["Examples"]["answer"]["everything"] = 42;

				//add an array stored as std::vector (using an initializer list)
				input["Examples"]["list"] = { 1, 0, 2 };

				//add another object (using an initializer list of pairs)
				input["Examples"]["object"] = { {"currency", "SGD"}, {"value", 42.99} };

				for (int i{}; i < num_objects_in_test_scene; ++i)
				{
					input[root][i]["Sorting_Order"] = i;
					input[root][i]["Transform"]["Position"] = { 1.0f, 2.0f, 3.0f };
					input[root][i]["Transform"]["Rotation"] = { 1.0f, 2.0f, 3.0f };
					input[root][i]["Transform"]["Scale"] = { 1.0f, 2.0f, 3.0f };
				}

				SerializeFile(input, testPath + std::string("JSONTest1.scene"));

				json output = DeserializeFile(testPath + std::string("JSONTest1.scene"));
				if (output == json())
				{
					SLICE_LOG_ERROR(testPath + std::string("JSONTest1.scene") + " is empty");
				}
				if (cleanOutput)
				{
					std::filesystem::remove(testPath + std::string("JSONTest1.scene"));
				}
				SLICE_LOG("Test 1 Ended.");
			}

			void Test2(bool cleanOutput)
			{
				SLICE_LOG("Test 2 Beginning...");
				auto& factory = FactoryInstance;

				GameObject omnia_victrum = factory.CreateGO("Omnia_Victrum");

				// Set up transform via the component system
				auto& transform = omnia_victrum.GetComponent<Transform>();
				transform.position = glm::vec3(1, 2, 3);
				transform.rotation = glm::vec3(4, 5, 6);
				transform.scale = glm::vec3(7, 8, 9);

				omnia_victrum.AddComponent<SceneGraph>();
				auto& scenegraph = omnia_victrum.GetComponent<SceneGraph>();
				scenegraph.entity_id = entt::to_integral(omnia_victrum.GetEntity());
				for (size_t i{}; i < SceneGraph::Direction::DIRECTIONS; ++i)
				{
					scenegraph.neighbours[i] = static_cast<Entity>(i);
				}

				// Serialize the object
				SerializeScene(testPath + std::string("JSONTest2.json"));

				factory.Destroy(omnia_victrum);

				DeserializeScene(testPath + std::string("JSONTest2.json"));
				if (cleanOutput)
				{
					std::filesystem::remove(testPath + std::string("JSONTest2.json"));
				}
				SLICE_LOG("Test 2 Ended.");
			}

			void Test3(bool cleanOutput)
			{
				SLICE_LOG("Test 3 Beginning...");
				auto& factory = FactoryInstance;
				GameObject parent = factory.CreateGO("Bing_Bong_Parent");
				GameObject child = factory.CreateGO("Bing_Bong_Child");

				// Set up transform via the component system
				auto& parentTransform = parent.GetComponent<Transform>();
				parentTransform.position = glm::vec3(10, 11, 12);
				parentTransform.rotation = glm::vec3(13, 14, 15);
				parentTransform.scale = glm::vec3(16, 17, 18);

				auto& childTransform = parent.GetComponent<Transform>();
				childTransform.position = glm::vec3(10, 11, 12);
				childTransform.rotation = glm::vec3(13, 14, 15);
				childTransform.scale = glm::vec3(16, 17, 18);

				parent.AddComponent<SceneGraph>();
				auto& parentScenegraph = parent.GetComponent<SceneGraph>();
				parentScenegraph.entity_id = entt::to_integral(parent.GetEntity());

				child.AddComponent<SceneGraph>();
				auto& childScenegraph = child.GetComponent<SceneGraph>();
				childScenegraph.entity_id = entt::to_integral(child.GetEntity());

				factory.SetParent(child.GetEntity(),parent.GetEntity());				

				SerializeScene(testPath + std::string("JSONTest3.json"));

				factory.Destroy(parent);
				factory.Destroy(child);

				DeserializeScene(testPath + std::string("JSONTest3.json"));
				if (cleanOutput)
				{
					std::filesystem::remove(testPath + std::string("JSONTest3.json"));
				}
				SLICE_LOG("Test 3 Ended.");
			}

			void Test4(bool cleanOutput)
			{

			}

			void RunTests(TestNum testNum, bool cleanOutput)
			{
				switch (testNum)
				{
				case TEST1:
					Test1(cleanOutput);
					break;

				case TEST2:
					Test2(cleanOutput);
					break;
				case TEST3:
					Test3(cleanOutput);
					break;
				case TEST4:
					Test4(cleanOutput);
					break;
				default:
					Test1(cleanOutput);
					Test2(cleanOutput);
					Test3(cleanOutput);
					break;
				}
				SLICE_LOG("Test(s) Completed, Examine Console Log for Errors");
			}
		}
	}
}