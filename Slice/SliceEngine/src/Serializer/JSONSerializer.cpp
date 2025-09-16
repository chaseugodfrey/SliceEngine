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

				//rttr::type rtype = EnttIdToRttrTypeFunc(type_id);
				//
				//// Get instance through registered getter				
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
		}

		void Test()
		{
			JSONSerializer::TestSerialize();
			JSONSerializer::TestDeserialize();
		}

		void Test2()
		{
			auto& manufactorum_ajakis = Core::GetInstance()->GetRegistry();

			//manufactorum_ajakis.MapEnttToRTTR<Transform>();
			//manufactorum_ajakis.CreateComponentCloner<Transform>();

			entt::entity omnia_victrum_entity = Core::GetInstance()->GetRegistry().create();

			Transform& t = manufactorum_ajakis.emplace<Transform>(omnia_victrum_entity);
			t.position = glm::vec3(1, 2, 3);
			t.rotation = glm::vec3(4, 5, 6);
			t.scale = glm::vec3(7, 8, 9);


			GameObject omnia_victrum = GameObject(manufactorum_ajakis, omnia_victrum_entity);

			//omnia_victrum.AddComponent<Transform>();
			//omnia_victrum.GetComponent<Transform>().position = glm::vec3(1, 2, 3);
			//omnia_victrum.GetComponent<Transform>().rotation = glm::vec3(4, 5, 6);
			//omnia_victrum.GetComponent<Transform>().scale = glm::vec3(7, 8, 9);

			Serialize(RecursiveSerialize(omnia_victrum),"tests/Imperial_Titans.json");

		}
	}
}