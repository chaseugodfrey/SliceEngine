#include <pch.h>
#include "JSONSerializer.h"

namespace SliceEngine
{
	namespace JSONSerializer
	{
		std::unordered_map<uint32_t, uint32_t> sceneGraphMap;

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
		void Serialize(json const& input, std::filesystem::path const& filePath)
		{
			std::ofstream ofs(filePath);
			if (!ofs.is_open())
			{
				Logger::LogError("JSONSerializer::Serialize", "Unable to write JSON to path: " + filePath.string());
				return;
			}
			ofs << input;
			ofs.close();
		}

		json SerializeGameObject(GameObject& node)
		{
			json output;

			auto& registry = Core::GetInstance()->GetRegistry();
			entt::entity entity = node.GetEntity();

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

					if (!propVal.is_valid())
					{
						continue;
					}

					if (propVal.is_type<int>())
					{
						output[node.GetName()][storage.type().name()][propName] = propVal.get_value<int>();
					}
					else if (propVal.is_type<float>())
					{
						output[node.GetName()][storage.type().name()][propName] = propVal.get_value<float>();
					}
					else if (propVal.is_type<double>())
					{
						output[node.GetName()][storage.type().name()][propName] = propVal.get_value<double>();
					}
					else if (propVal.is_type<uint32_t>())
					{
						output[node.GetName()][storage.type().name()][propName] = propVal.get_value<uint32_t>();
					}
					if (propVal.get_type().is_derived_from(rttr::type::get<EntityID>()) ||
						propVal.get_type() == rttr::type::get<EntityID>())
					{
						EntityID eid = propVal.get_value<EntityID>();
						output[node.GetName()][storage.type().name()][propName] = eid.value;
					}
					else if (propVal.is_type<std::array<uint32_t, 4>>())
					{
						const auto& vec = propVal.get_value<std::array<uint32_t, 4>>();
						for (size_t i{}; i < 4; ++i)
						{
							output[node.GetName()][storage.type().name()][propName][i] = vec[i];
						}
					}
					else if (propVal.is_type<std::array<Entity, 4>>())
					{
						const auto& vec = propVal.get_value<std::array<Entity, 4>>();
						for (size_t i{}; i < 4; ++i)
						{
							output[node.GetName()][storage.type().name()][propName][i] = vec[i];
						}
					}
					else if (propVal.is_type<std::string>())
					{
						output[node.GetName()][storage.type().name()][propName] = propVal.get_value<std::string>();
					}
					else if (propVal.is_type<std::vector<uint32_t>>())
					{
						const auto& vec = propVal.get_value<std::vector<uint32_t>>();
						for (const auto& elem : vec)
						{
							output[node.GetName()][storage.type().name()][propName].push_back(elem);
						}
					}

					else if (propVal.is_type<glm::vec3>())
					{
						auto v = propVal.get_value<glm::vec3>();
						output[node.GetName()][storage.type().name()][propName] = { v.x, v.y, v.z };
					}
					else
					{
						// fallback
						output[node.GetName()][storage.type().name()][propName] = propVal.to_string();
					}
				}
			}

			return output;
		}

		//Loads JSON (the file) and returns it as a json (the data structure) that can be accessed and edited
		json Deserialize(std::filesystem::path const& filePath)
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

		void DeserializeGameObjects(json const& input)
		{
			for (auto& [name, components] : input.items())
			{
				auto& factory = Core::GetInstance()->mFactory;
				GameObject node = factory.CreateBlank();

				// Temprorary until have createGO without transform
				//node.RemoveComponent<Transform>();

				for (auto& [componentName, props] : components.items())
				{
					rttr::type compType = rttr::type::get_by_name(componentName);
					if (!compType)
					{
						SLICE_LOG_ERROR(componentName + " not registered");
						continue;
					}

					rttr::variant componentInstance = compType.create();
					if (!componentInstance.is_valid())
					{
						SLICE_LOG_ERROR("Failed to create instance of " + componentName);
						continue;
					}

					//Note for hafiz and me: I moved the mName to sliceentity component
					// technically there should never be an instance of two objects with the same name serialized
					// since factory checks for that
					// So i shouldn't have to check for duplicate names when deserializing
					// but keep a note incase it dies next time

					for (auto& [propName, value] : props.items())
					{
						rttr::property prop = compType.get_property(propName);
						if (!prop.is_valid())
							continue;

						if (prop.get_type() == rttr::type::get<int>())
							prop.set_value(componentInstance, value.get<int>());
						else if (prop.get_type() == rttr::type::get<float>())
							prop.set_value(componentInstance, value.get<float>());
						else if (prop.get_type() == rttr::type::get<double>())
							prop.set_value(componentInstance, value.get<double>());
						else if (prop.get_type() == rttr::type::get<uint32_t>())
							prop.set_value(componentInstance, value.get<uint32_t>());
						else if (prop.get_type() == rttr::type::get<std::string>())
							prop.set_value(componentInstance, value.get<std::string>());
						else if (prop.get_type() == rttr::type::get<std::vector<uint32_t>>())
						{
							std::vector<uint32_t> vec;
							for (auto& v : value)
							{
								vec.push_back(v.get<uint32_t>());
							}
							prop.set_value(componentInstance, vec);
						}
						else if (prop.get_type() == rttr::type::get<EntityID>())
						{
							uint32_t rawID = value.get<uint32_t>();
							prop.set_value(componentInstance, EntityID{ rawID });
						}
						else if (prop.get_type() == rttr::type::get<glm::vec3>())
						{
							glm::vec3 vec{ value[0].get<float>(), value[1].get<float>(), value[2].get<float>() };
							prop.set_value(componentInstance, vec);
						}
						else if (prop.get_type() == rttr::type::get<std::array<uint32_t, 4>>())
						{
							std::array<uint32_t, 4> arr;
							arr = value;
							prop.set_value(componentInstance, arr);
						}
						else if (prop.get_type() == rttr::type::get<std::array<Entity, 4>>())
						{
							std::array<Entity, 4> arr;
							arr = value;
							prop.set_value(componentInstance, arr);
						}
						else
						{
							// fallback: try string
							prop.set_value(componentInstance, value.get<std::string>());
						}

						if (compType == rttr::type::get<SceneGraph>())
						{
							auto& sg = componentInstance.get_value<SceneGraph>();

							sceneGraphMap[sg.entity_id.value] = entt::to_integral(node.GetEntity());
							SLICE_LOG("Old ID " + std::to_string(sg.entity_id.value) + " Mapped to new ID " + std::to_string(entt::to_integral(node.GetEntity())));
						}
					}

					AddComponentFromVariant(node, componentInstance, componentName);										
				}
			}
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

				Serialize(input, testPath + std::string("JSONTest1.scene"));

				json output = Deserialize(testPath + std::string("JSONTest1.scene"));
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
				Serialize(SerializeGameObject(omnia_victrum), testPath + std::string("JSONTest2.json"));

				factory.Destroy(omnia_victrum);

				DeserializeGameObjects(Deserialize(testPath + std::string("JSONTest2.json")));
				if (cleanOutput)
				{
					std::filesystem::remove(testPath + std::string("JSONTest2.json"));
				}
				SLICE_LOG("Test 2 Ended.");
			}

			void RunTests(bool cleanOutput)
			{
				Test1(cleanOutput);
				Test2(cleanOutput);
				SLICE_LOG("Tests Completed, Examine Console Log for Errors");
			}
		}
	}
}