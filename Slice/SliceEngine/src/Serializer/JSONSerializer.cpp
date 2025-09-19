#include <pch.h>
#include "JSONSerializer.h"

namespace SliceEngine
{


	namespace JSONSerializer
	{
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

		json RecursiveSerialize(GameObject& node)
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
					SLICE_LOG_ERROR("Component is not registered");
					continue;
				}

				auto it = Core::GetInstance()->mFactory.mComponentGetters.find(type_id);
				if (it == Core::GetInstance()->mFactory.mComponentGetters.end())
				{
					SLICE_LOG_ERROR("Component does not have a getter");
					// assert?

					continue;
				}

				rttr::variant componentData = it->second(registry, entity);

				for (const auto& property : componentType.get_properties())
				{
					// this should be the component's property data
					std::string propName = property.get_name().to_string();

					rttr::variant propVal = property.get_value(componentData);

					// idk if this works tho
					// nvm it works
				}

				//rttr::type rtype = EnttIdToRttrTypeFunc(type_id);
				
				// Get instance through registered getter				
				//rttr::instance inst = InstanceGetterFunc[type_id](registry, entity);

				//if (!inst.is_valid()) continue;

				//// Now you can reflect over properties
				//for (auto& prop : rtype.get_properties()) 
				//{
				//	rttr::variant value = prop.get_value(inst);

				//	//std::cout << prop.get_name() << " = " << value.to_string() << std::endl;

				//	output[std::string(rtype.get_name())][std::string(prop.get_name())] = value.to_string();
				//}

			}

			// If you have children, recurse:
			//for (auto& child : ) {
			//	output["children"].push_back(RecursiveSerialize(child));
			//}

			return output;
		}

		json SerializeEntt(entt::entity entity)
		{
			json output;
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

		GameObject RecursiveDeserialize(json const& input)
		{
			return GameObject();
		}

		static void TestSerialize()
		{
			json test;

			std::string root("Scene");
			int num_objects_in_test_scene{ 10 };

			//add a number stored as double
			test["Examples"]["pi"] = 3.141;

			//add a Boolean stored as bool
			test["Examples"]["happy"] = true;

			//add a string stored as std::string
			test["Examples"]["name"] = "Niels";

			//add another null object by passing nullptr
			test["Examples"]["nothing"] = nullptr;

			//add an object inside the object
			test["Examples"]["answer"]["everything"] = 42;

			//add an array stored as std::vector (using an initializer list)
			test["Examples"]["list"] = { 1, 0, 2 };

			//add another object (using an initializer list of pairs)
			test["Examples"]["object"] = { {"currency", "SGD"}, {"value", 42.99} };

			for (int i{}; i < num_objects_in_test_scene; ++i)
			{
				test[root][i]["Sorting_Order"] = i;
				test[root][i]["Transform"]["Position"] = { 1.0f, 2.0f, 3.0f };
				test[root][i]["Transform"]["Rotation"] = { 1.0f, 2.0f, 3.0f };
				test[root][i]["Transform"]["Scale"] = { 1.0f, 2.0f, 3.0f };
			}

			Serialize(test, "Assets/Scenes/JSONTest1.scene");
		}

		static void TestDeserialize()
		{
			json test = Deserialize("Assets/Scenes/JSONTest1.scene");
			if (test != json())
			{
				Logger::LogValue("TestDeserialize", "Test Successful");
			}
		}

		void Test()
		{
			JSONSerializer::TestSerialize();
			JSONSerializer::TestDeserialize();
		}

		void Test2()
		{
			auto& factory = Core::GetInstance()->mFactory;

			// Ensure RTTR mapping + cloners exist
			//factory.MapEnttToRTTR<Transform>();
			//factory.CreateComponentCloner<Transform>();

			// Use factory to create a game object (instead of raw registry)
			GameObject omnia_victrum = factory.CreateGO("Omnia_Victrum");

			// Set up transform via the component system
			auto& t = omnia_victrum.GetComponent<Transform>();
			t.position = glm::vec3(1, 2, 3);
			t.rotation = glm::vec3(4, 5, 6);
			t.scale = glm::vec3(7, 8, 9);

			// Serialize the object
			Serialize(RecursiveSerialize(omnia_victrum), "Imperial_Titans.json");
		}
	}
}