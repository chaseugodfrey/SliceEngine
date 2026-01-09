/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			JSONSerializer.cpp
 author:		Hafiz
 email:			b.muhammadhafiz@digipen.edu
 brief:			Serialize and Deserialize JSON data

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "JSONSerializer.h"
#include "Animator/BoneSystem.h"
#include "Scripting/ScriptSystem.h"

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
			auto* rc = Core::GetInstance()->GetResourceManager();
			rc->mGUIDToSerialize.clear(); // clear it all first to make sure its empty
			auto entityView = registry.view<SliceEntity>();
			for (auto entity : entityView)
			{
				output += SerializeGameObject(entity, registry);
			}

			SerializeFile(output, filePath);

			json GUIDFile = SerializeSceneResources();

			std::filesystem::path outPath(filePath);
			std::string resourcePath = outPath.replace_extension(".resource").string();

			SerializeFile(GUIDFile, resourcePath);
		}

		json SerializeSceneResources()
		{
			auto& registry = Core::GetInstance()->GetRegistry();
			auto resourceManager = Core::GetInstance()->GetResourceManager();
			auto entityView = registry.view<SliceEntity>();

			// Clean up the set before serializing
			resourceManager->mGUIDToSerialize.clear();
			for (auto entity : entityView)
			{
				FactoryInstance.VisitComponents(entity, [&resourceManager](rttr::type type, rttr::variant& component)
					{
						for (const auto& property : type.get_properties())
						{
							// this should be the component's property data
							std::string propName = property.get_name().to_string();

							rttr::variant propVal = property.get_value(component);

							rttr::type propType = propVal.get_type();

							// pull out all the GUIDs when looping through the components
							if (propType == rttr::type::get<GUID>())
							{
								resourceManager->mGUIDToSerialize.insert(propVal.get_value<GUID>());
							}

							// for now, we'll ignore resources in c# scripts cause i have to loop through the scriptable data map
						}
					});
			}

			json GUIDFile;

			for (auto guid : resourceManager->mGUIDToSerialize)
			{
				GUIDFile += guid.GetGUID();
			}

			return GUIDFile;
		}

		// idk what would be passed in when deserializing in scene system
		void DeserializeSceneResource(std::filesystem::path const& filePath)
		{

			std::filesystem::path outPath(filePath);
			std::string resourcePath = outPath.replace_extension(".resource").string();

			json sceneResource = DeserializeFile(resourcePath);

			for (auto guid : sceneResource.items())
			{

			}
		}

#pragma region PrefabSerializing
		std::string SerializePrefab(entt::entity entity)
		{
			json output;
			auto& registry = Core::GetInstance()->GetRegistry();

			// serialize the main entity
			output += SerializeGameObject(entity, registry);

			// Now serialize the children
			auto& sceneGraph = registry.get<SliceEngine::SceneGraph>(entity);
			auto childEntity = sceneGraph.neighbours[SceneGraph::DOWN];
			
			while (childEntity != entt::null)
			{
				SerializePrefabChild(output, childEntity, registry);
				auto& childEntityGraph = registry.get<SceneGraph>(childEntity);
				childEntity = childEntityGraph.neighbours[SceneGraph::RIGHT];
			}

			// TODO: Find out a better way we shud be doing this
			std::filesystem::path mAssetDirectory = std::filesystem::path("Assets");
			std::filesystem::path filePath = mAssetDirectory.string() + "/" + "Prefabs" + "/" + registry.get<SliceEntity>(entity).mName + ".prefab";
			
			// if the prefab already exist, then it means we're modifying/updating an existing prefab
			// so we need to check if there was any change in the objects created
			// i need to find out how to retrieve all the components of gameobjects to store in a map
			// prefabID to list of components, then compare that to the new one being serialized
			// Store the ones taht are new and deleted in two different maps
			// send that info to prefab system to handle updating existing prefab instances in the scene

			
			SerializeFile(output, filePath);

			return filePath.string();
		}

		// Looks kinda funky having two exact same functions
		// but 1 is the main function that starts the recursive call from the original main prefab entity
		// the second one is for iterating the children
		// it'll check down and right
		// and go down again if there is one
		// hopefully our prefabs not gonna be so complicated that this whole recursive call is long and breakable

		void SerializePrefabChild(json& output, entt::entity entity, entt::registry& registry)
		{
			output += SerializeGameObject(entity, registry);
			auto& sceneGraph = registry.get<SliceEngine::SceneGraph>(entity);
			auto childEntity = sceneGraph.neighbours[SceneGraph::DOWN];

			while (childEntity != entt::null)
			{
				SerializePrefabChild(output, childEntity, registry);
				auto& childEntityGraph = registry.get<SceneGraph>(childEntity);
				childEntity = childEntityGraph.neighbours[SceneGraph::RIGHT];
			}

		}

		Entity DeserializePrefab(std::filesystem::path const& filePath, bool Editor)
		{
			std::unordered_map<uint32_t, uint32_t> sceneGraphMap{};
			std::vector<Entity> entityID;
			// TODO: ask hafiz if theres a btr way for this
			// im just gonna duck tape this for now
			std::vector<rttr::variant> delayedComponentInstance;
			std::vector<std::string> delayedComponentName;
			std::vector<GameObject> delayedGO;

			json prefab = DeserializeFile(filePath);
			auto& factory = Core::GetInstance()->mFactory;

			for (auto& [name, components] : prefab.items())
			{
				GameObject newObj = factory.CreateBlanker();
				if(Editor)
					newObj.AddComponent<PrefabEditingEntity>();
				//factory.RemoveFromNameMap(newObj.GetEntity());
				//factory.PrintNameMap();
			//std::string goName = factory.GetNameFromMap(newObj.GetEntity());
				//factory.PrintNameMap();
				entityID.push_back(newObj.GetEntity());
				for (auto& [objName, objProps] : components.items())
				{
					for (auto& [componentName, props] : objProps.items())
					{
						rttr::type compType = rttr::type::get_by_name(componentName);
						if (!compType)
						{
							continue;
						}

						rttr::variant componentInstance = compType.create();
						if (!componentInstance.is_valid())
						{
							continue;
						}

						for (auto& [propName, value] : props.items())
						{
							rttr::property prop = compType.get_property(propName);

							//factory.PrintNameMap();

							if (!prop.is_valid())
								continue;

							DeserializeProp
								<
								int,
								unsigned int,
								unsigned char,
								float,
								double,
								bool,
								Entity,
								uint32_t,
								uint64_t,
								GUID,
								Handle<SliceEngineTypes::Texture>,
								Handle<SliceEngineTypes::Model>,
								Handle<SliceEngineTypes::Material>,
								Handle<SliceEngineTypes::Skeleton>,
								Handle<SliceEngineTypes::AnimationPackage>,
								Handle<SliceEngineTypes::StateMachine>,
								Handle<SliceEngineTypes::Prefab>,
								std::array<uint64_t, 4>,
								std::array<Entity, 4>,
								std::vector<uint64_t>,
								glm::vec2,
								glm::vec3,
								glm::vec4,
								glm::quat,
								std::string,
								std::unordered_map<std::string, rttr::variant>,
								JPH::Vec3,
								ColliderShape::BoxData,
								ColliderShape::SphereData,
								ColliderShape::CapsuleData,
								RigidBody::FreezeOptions
								>
								(componentInstance, prop, value, propName, componentName, newObj.GetEntity());
							// Anything that needs a second pass
							// scene graph map stuff
							if (propName == "entity_id" && componentName == typeid(SceneGraph).name())
							{
								uint32_t oldID = value.get<uint32_t>();
								sceneGraphMap[oldID] = entt::to_integral(newObj.GetEntity());
							}
						}

						// if its a script component, dont add it now
						if (componentName == typeid(Script).name())
						{
							delayedComponentInstance.push_back(componentInstance);
							delayedComponentName.push_back(componentName);
							delayedGO.push_back(newObj);
							continue;
						}

						AddComponentFromVariant(newObj, componentInstance, componentName);
					}
				}
			}

			//factory.PrintNameMap();

			// if its cloning an object, creating a prefab of an object
			// and that object exist in the scene
			// it would end up havint the same name
			// which will break
			auto& registry = Core::GetInstance()->GetRegistry();

			// idfk why using sceneGraphMap begin breaks if its creating objects made from fbx
			// it works for any other type of prefabs 
			// i just get the first entity in entityID instead cause thats the first obj created
			auto rootEntity = entityID[0];
			GameObject rootGO = factory.GetGOByEntity(rootEntity);
			// the root prefab gameobject, should have no siblings as it's gonna be inserted into the graph
			if (registry.any_of<SceneGraph>(rootEntity))
			{
				auto& sceneGraph = registry.get<SceneGraph>(rootEntity);

				// it becomes a root for now, so it wont have an up, left or right
				sceneGraph.neighbours[SceneGraph::LEFT] = entt::null;
				sceneGraph.neighbours[SceneGraph::RIGHT] = entt::null;
				sceneGraph.neighbours[SceneGraph::UP] = entt::null;
			}

			if (rootGO.HasComponent<SliceEntity>())
			{
				rootGO.GetComponent<SceneGraph>().entity_id = (uint32_t)rootGO.GetEntity();
			}

			// fix the old to new entity IDs
			for (auto entity : entityID)
			{
				if (!registry.any_of<SceneGraph>(entity))
				{
					continue;
				}

				auto& sceneGraphComponent = registry.get<SceneGraph>(entity);
				sceneGraphComponent.entity_id = (unsigned int)entity;
				for (int i = 0; i < sceneGraphComponent.neighbours.size(); ++i)
				{
					auto it = sceneGraphMap.find((uint64_t)sceneGraphComponent.neighbours[i]);
					if (it != sceneGraphMap.end())
					{
						sceneGraphComponent.neighbours[i] = (Entity)it->second;
					}
				}
				//factory.PrintNameMap();
				if (!Editor)
				{
					//factory.PrintNameMap();
					//std::string goName = factory.GetNameFromMap(entity);

					// handle adding to name map here
					FactoryInstance.AddToNameMap(entity);
					//factory.PrintNameMap();
					//	goName = factory.GetNameFromMap(entity);
				}
				//factory.PrintNameMap();

			}

			// only once all the fixing of entity IDs and stuff is done, then we add the component
			for (size_t i = 0; i < delayedComponentInstance.size(); ++i)
			{
				AddComponentFromVariant(delayedGO[i], delayedComponentInstance[i], delayedComponentName[i]);
			}

			for (auto entity : entityID)
			{
				if (!registry.any_of<Bone>(entity))
				{
					continue;
				}

				auto& boneComponent = registry.get<Bone>(entity);
				boneComponent.skeleton_root = (Entity)sceneGraphMap[(uint32_t)boneComponent.skeleton_root];
			}

			for (auto entity : entityID)
			{
				if (!registry.any_of<Bone>(entity))
				{
					continue;
				}

				Core::GetInstance()->GetSystem<BoneSystem>().Update_Bones(registry, entity);
			}

			if (rootGO.HasComponent<Slider>()) {	//handle and fill entity remapping for slider
				auto entityView = registry.view<Slider>();
				for (auto entity : entityView) {
					auto& slider = registry.get<Slider>(entity);

					slider.fill = (Entity)sceneGraphMap[(uint32_t)slider.fill];
					slider.handle = (Entity)sceneGraphMap[(uint32_t)slider.handle];
				}
			}


			return rootEntity;
		}
		std::unordered_map<unsigned int, std::vector<rttr::variant>> DeserializePrefabComponents(std::filesystem::path const& filePath)
		{
			std::unordered_map<unsigned int, std::vector<rttr::variant>> componentInstances;

			json prefab = DeserializeFile(filePath);
			auto& factory = Core::GetInstance()->mFactory;

			for (auto& [name, components] : prefab.items())
			{
				std::vector<rttr::variant> variantComponents;
				uint32_t prefabID = UINT_MAX;
				for (auto& [objName, objProps] : components.items())
				{
					for (auto& [componentName, props] : objProps.items())
					{
						rttr::type compType = rttr::type::get_by_name(componentName);
						if (!compType)
						{
							continue;
						}

						rttr::variant componentInstance = compType.create();
						if (!componentInstance.is_valid())
						{
							continue;
						}

						for (auto& [propName, value] : props.items())
						{
							rttr::property prop = compType.get_property(propName);

							//factory.PrintNameMap();

							if (!prop.is_valid())
								continue;

							DeserializeProp
								<
								int,
								unsigned int,
								unsigned char,
								float,
								double,
								bool,
								Entity,
								uint32_t,
								uint64_t,
								GUID,
								Handle<SliceEngineTypes::Texture>,
								Handle<SliceEngineTypes::Model>,
								Handle<SliceEngineTypes::Material>,
								Handle<SliceEngineTypes::Skeleton>,
								Handle<SliceEngineTypes::AnimationPackage>,
								Handle<SliceEngineTypes::StateMachine>,
								Handle<SliceEngineTypes::Prefab>,
								std::array<uint64_t, 4>,
								std::array<Entity, 4>,
								std::vector<uint64_t>,
								glm::vec2,
								glm::vec3,
								glm::vec4,
								glm::quat,
								std::string,
								std::unordered_map<std::string, rttr::variant>,
								JPH::Vec3,
								ColliderShape::BoxData,
								ColliderShape::SphereData,
								ColliderShape::CapsuleData,
								RigidBody::FreezeOptions
								>
								(componentInstance, prop, value, propName, componentName, (Entity)0);

							if (propName == "prefabID" && componentName == typeid(Prefab).name())
							{
								prefabID = value.get<uint32_t>();
							}

						}


						if (componentName != typeid(SliceEntity).name() &&
							componentName != typeid(Transform).name() &&
							componentName != typeid(SceneGraph).name() &&
							componentName != typeid(Prefab).name() &&
							componentName != typeid(Bone).name())
						{
							variantComponents.push_back(componentInstance);
						}

						
						//AddComponentFromVariant(newObj, componentInstance, componentName);
					}
				}

				if (prefabID != UINT_MAX)
					componentInstances[prefabID] = variantComponents;
			}
			return componentInstances;
		}
#pragma endregion

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
					//SLICE_LOG_ERROR(std::string(storage.type().name()) + " is not registered");
					continue;
				}

				// NOTE: Any components that you want to be serialized last
				// then skip them here
				// script has to be done last cause some construct/create functions retrieve other components
				if (componentType == rttr::type::get<Script>())
				{
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

					if (componentType == rttr::type::get<ColliderShape>())
					{
						size_t activeIndex = componentData.get_value<ColliderShape>().shapeData.index();

						if ((propName == "boxData" && activeIndex != 0) ||
							(propName == "sphereData" && activeIndex != 1) ||
							(propName == "capsuleData" && activeIndex != 2))
						{
							continue;
						}
					}

					rttr::variant propVal = property.get_value(componentData);

					std::string name = FactoryInstance.GetGOByEntity(entity).GetName();

					if (!propVal.is_valid())
					{
						continue;
					}

					//if (propVal.get_type() == rttr::type::get<GUID>())
					//{
					//	Core::GetInstance()->GetResourceManager()->mGUIDToSerialize.insert(propVal.get_value<GUID>());
					//}



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
						Entity,
						uint32_t,
						uint64_t,
						GUID,
						Handle<SliceEngineTypes::Texture>,
						Handle<SliceEngineTypes::Model>,
						Handle<SliceEngineTypes::Material>,
						Handle<SliceEngineTypes::Skeleton>,
						Handle<SliceEngineTypes::AnimationPackage>,
						Handle<SliceEngineTypes::StateMachine>,
						Handle<SliceEngineTypes::Prefab>,
						std::array<uint64_t, 4>,
						std::array<Entity, 4>,
						std::vector<uint64_t>,
						glm::vec2,
						glm::vec3,
						glm::vec4,
						glm::quat,
						std::string,
						std::unordered_map<std::string, rttr::variant>,
						JPH::Vec3,
						ColliderShape::BoxData,
						ColliderShape::SphereData,
						ColliderShape::CapsuleData,
						RigidBody::FreezeOptions,
						GameObject
						>
						(output, name, storage.type().name(), propName, propVal, static_cast<Entity>(entity));
				}
			}

			// for now jus scripts
			// idk if i have to do this for more stuff
			if (registry.any_of<Script>(entity))
			{
				entt::id_type type_id = entt::type_id<Script>().hash();
				auto it = Core::GetInstance()->mFactory.mComponentGetters.find(type_id);
				if (it != Core::GetInstance()->mFactory.mComponentGetters.end())
				{
					rttr::variant componentData = it->second(registry, entity);
					rttr::type componentType = rttr::type::get<Script>();

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

						SerializeProp
							<
							int,
							unsigned int,
							unsigned char,
							float,
							double,
							bool,
							Entity,
							uint32_t,
							uint64_t,
							GUID,
							Handle<SliceEngineTypes::Texture>,
							Handle<SliceEngineTypes::Model>,
							Handle<SliceEngineTypes::Material>,
							Handle<SliceEngineTypes::Skeleton>,
							Handle<SliceEngineTypes::AnimationPackage>,
							Handle<SliceEngineTypes::StateMachine>,
							Handle<SliceEngineTypes::Prefab>,
							std::array<uint64_t, 4>,
							std::array<Entity, 4>,
							std::vector<uint64_t>,
							glm::vec2,
							glm::vec3,
							glm::vec4,
							glm::quat,
							std::string,
							std::unordered_map<std::string, rttr::variant>,
							JPH::Vec3,
							ColliderShape::BoxData,
							ColliderShape::SphereData,
							ColliderShape::CapsuleData,
							RigidBody::FreezeOptions,
							GameObject
							>
							(output, name, componentType.get_name().to_string(), propName, propVal, static_cast<Entity>(entity));
					}

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
			DeserializeSceneResource(filePath);


			std::unordered_map<uint32_t, uint32_t> sceneGraphMap{};
			std::vector<Entity> entityID;
			json input = DeserializeFile(filePath);
			for (auto& [name, components] : input.items())
			{
				auto& factory = Core::GetInstance()->mFactory;
				GameObject node = factory.CreateBlank();
				// save every entity that is being created
				entityID.push_back(node.GetEntity());
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
								Entity,
								uint32_t,
								uint64_t,
								GUID,
								Handle<SliceEngineTypes::Texture>,
								Handle<SliceEngineTypes::Model>,
								Handle<SliceEngineTypes::Material>,
								Handle<SliceEngineTypes::Skeleton>,
								Handle<SliceEngineTypes::AnimationPackage>,
								Handle<SliceEngineTypes::StateMachine>,
								Handle<SliceEngineTypes::Prefab>,
								std::array<uint64_t, 4>,
								std::array<Entity, 4>,
								std::vector<uint64_t>,
								glm::vec2,
								glm::vec3,
								glm::vec4,
								glm::quat,
								std::string,
								std::unordered_map<std::string, rttr::variant>,
								JPH::Vec3,
								ColliderShape::BoxData,
								ColliderShape::SphereData,
								ColliderShape::CapsuleData,
								RigidBody::FreezeOptions
								>
								(componentInstance, prop, value, propName, componentName, node.GetEntity());

							// Anything that needs a second pass
							// scene graph map stuff
							if (propName == "entity_id" && componentName == typeid(SceneGraph).name())
							{
								uint32_t oldID = value.get<uint32_t>();
								sceneGraphMap[oldID] = entt::to_integral(node.GetEntity());
							}

							if (propName == "mName" && componentName == typeid(SliceEntity).name())
							{
								FactoryInstance.UpdateName(value, node.GetEntity());
							}
						}



						AddComponentFromVariant(node, componentInstance, componentName);
					}

				}
			}

			// Remapping Entity IDs after all GOs have been deserialized
			auto& registry = Core::GetInstance()->GetRegistry();
			for (auto entity : entityID)
			{
				if (!registry.any_of<Bone>(entity))
				{
					continue;
				}

				auto& boneComponent = registry.get<Bone>(entity);
				boneComponent.skeleton_root = (Entity)sceneGraphMap[(uint32_t)boneComponent.skeleton_root];
			}

			for (auto entity : entityID)
			{
				if (!registry.any_of<Bone>(entity))
				{
					continue;
				}

				Core::GetInstance()->GetSystem<BoneSystem>().Update_Bones(registry, entity);
			}

			//remap for slider
			auto sliderView = registry.view<Slider>();
			for (auto entity : sliderView) {
				auto& slider = registry.get<Slider>(entity);

				slider.fill = (Entity)sceneGraphMap[(uint32_t)slider.fill];
				slider.handle = (Entity)sceneGraphMap[(uint32_t)slider.handle];
			}

			gScriptSystem->RemapGameObjectVariables(sceneGraphMap);

			// Using scene graph map to fix scenegraph component is done in another function in scene system.

			return sceneGraphMap;
		}

		/// <summary>
		/// Used to convert variant elements into json format
		/// mostly only for script component since its a map of string to variants
		/// </summary>
		/// <param name="v">variant data</param>
		/// <returns>nlohman array or value </returns>
		nlohmann::json GetJsonFromVariant(rttr::variant v)
		{
			rttr::type t = v.get_type();
			if (t.is_wrapper())
			{
				v = v.extract_wrapped_value();

				t = v.get_type();
			}

			if (t.is_sequential_container())
			{
				auto view = v.create_sequential_view();
				nlohmann::json jArray = nlohmann::json::array();

				for (size_t i = 0; i < view.get_size(); ++i)
				{
					jArray.push_back(GetJsonFromVariant(view.get_value(i)));
				}

				return jArray;
			}

			if (t == rttr::type::get<glm::vec3>()) { return v.get_value<glm::vec3>(); }
			if (t == rttr::type::get<glm::vec2>()) { return v.get_value<glm::vec2>(); }
			if (t == rttr::type::get<JPH::Vec3>()) { return v.get_value<JPH::Vec3>(); }
			if (t == rttr::type::get<float>()) { return v.get_value<float>(); }
			if (t == rttr::type::get<int>()) { return v.get_value<int>(); }
			if (t == rttr::type::get<double>()) { return v.get_value<double>(); }
			if (t == rttr::type::get<char>()) { return v.get_value<char>(); }
			if (t == rttr::type::get<bool>()) { return v.get_value<bool>(); }
			if (t == rttr::type::get<unsigned int>()) { return v.get_value<unsigned int>(); }
			if (t == rttr::type::get<short>()) { return v.get_value<short>(); }
			if (t == rttr::type::get<std::string>()) { return v.get_value<std::string>(); }
			if (t == rttr::type::get<GameObject>()) 
			{ 
				Entity testVal = v.get_value<GameObject>().GetEntity();
				return v.get_value<GameObject>().GetEntity();
			}

			// fall back is to return as a string
			return v.to_string();
		}

		/// <summary>
		/// extracts the type out from variant and converts the value using GetJsonFromVariant
		/// </summary>
		/// <param name="v">variant</param>
		/// <returns></returns>
		nlohmann::json VariantToJson(rttr::variant v)
		{
			rttr::type t = v.get_type();
			std::string test2 = t.get_name().to_string();
			if (t.is_wrapper())
			{
				v = v.extract_wrapped_value();
				t = v.get_type();
			}

			nlohmann::json jsonOut = nlohmann::json::object();

			std::string test = t.get_name().to_string();
			jsonOut["Type"] = t.get_name().to_string();
			jsonOut["Value"] = GetJsonFromVariant(v);

			return jsonOut;
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

				factory.SetParent(child.GetEntity(), parent.GetEntity());

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