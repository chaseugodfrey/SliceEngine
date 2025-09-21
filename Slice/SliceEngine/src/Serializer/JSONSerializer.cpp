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

#define ADD_COMPONENT_CASE(T) \
    if (t == rttr::type::get<T>()) { \
        node.AddComponent<T>(componentInstance.get_value<T>()); \
        return true; \
    }

			ADD_COMPONENT_CASE(Transform)
			{
				//things
			};
			ADD_COMPONENT_CASE(SceneView);

#undef ADD_COMPONENT_CASE

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

				// Each component for this GameObject is here
				std::cout << storage.type().name() << std::endl;
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
					else if (propVal.is_type<uint64_t>())
					{
						output[node.GetName()][storage.type().name()][propName] = propVal.get_value<uint64_t>();
					}
					else if (propVal.is_type<std::string>())
					{
						output[node.GetName()][storage.type().name()][propName] = propVal.get_value<std::string>();
					}
					else if (propVal.is_type<std::vector<uint64_t>>())
					{
						const auto& vec = propVal.get_value<std::vector<uint64_t>>();
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

		std::vector<GameObject> DeserializeGameObjects(json const& input)
		{
			std::vector<GameObject> gameObjects;

			for (auto& [name, components] : input.items())
			{
				GameObject node;
				node.SetName(name);

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

					for (auto& [propName, value] : props.items())
					{
						rttr::property prop = compType.get_property(propName);
						if (!prop.is_valid())
							continue;

						if (prop.get_type() == rttr::type::get<int>())
							prop.set_value(componentInstance, value.get<int>());
						else if (prop.get_type() == rttr::type::get<float>())
							prop.set_value(componentInstance, value.get<float>());
						else if (prop.get_type() == rttr::type::get<uint64_t>())
							prop.set_value(componentInstance, value.get<uint64_t>());
						else if (prop.get_type() == rttr::type::get<std::string>())
							prop.set_value(componentInstance, value.get<std::string>());
						else if (prop.get_type() == rttr::type::get<std::vector<uint64_t>>())
						{
							std::vector<uint64_t> vec;
							for (auto& v : value)
								vec.push_back(v.get<uint64_t>());
							prop.set_value(componentInstance, vec);
						}
						else if (prop.get_type() == rttr::type::get<glm::vec3>())
						{
							glm::vec3 vec{ value[0].get<float>(), value[1].get<float>(), value[2].get<float>() };
							prop.set_value(componentInstance, vec);
						}
						else
						{
							// fallback: try string
							prop.set_value(componentInstance, value.get<std::string>());
						}
					}

					// Attach component to GameObject (depends on your ECS)
					AddComponentFromVariant(node, componentInstance, componentName);
				}

				gameObjects.push_back(std::move(node));
			}

			return gameObjects;
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
				SLICE_LOG("Test 1 Ended...");
			}

			void Test2(bool cleanOutput)
			{
				SLICE_LOG("Test 2 Beginning...");
				auto& factory = Core::GetInstance()->mFactory;

				GameObject omnia_victrum = factory.CreateGO("Omnia_Victrum");

				// Set up transform via the component system
				auto& transform = omnia_victrum.GetComponent<Transform>();
				transform.position = glm::vec3(1, 2, 3);
				transform.rotation = glm::vec3(4, 5, 6);
				transform.scale = glm::vec3(7, 8, 9);

				omnia_victrum.AddComponent<SceneView>();

				auto& sceneview = omnia_victrum.GetComponent<SceneView>();
				sceneview.parentGUID = 1234;
				for (int i{}; i < 5; ++i)
				{
					sceneview.childrenGUID.push_back(i);
				}
				sceneview.upGUID = 56;
				sceneview.downGUID = 78;

				// Serialize the object
				Serialize(SerializeGameObject(omnia_victrum), testPath + std::string("JSONTest2.json"));				
				DeserializeGameObjects(Deserialize(testPath + std::string("JSONTest2.json")));
				if (cleanOutput)
				{
					std::filesystem::remove(testPath + std::string("JSONTest2.json"));
				}
				SLICE_LOG("Test 2 Ended...");
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