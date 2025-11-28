#include <pch.h>
#include "PrefabSystem.h"
#include "Core/Core.h"
#include "Serializer/JSONSerializer.h"
#include "Scripting/ScriptSystem.h"

namespace SliceEngine
{
	void PrefabSystem::EntityOnEnter(entt::registry& reg, entt::entity entity)
	{
		// TODO: When an entity comes into the system
		// check if the prefab it references exists/is modified
		// if not then unprefab it (remove prefab component)
		GameObject GO = FactoryInstance.GetGOByEntity(entity);
		Prefab& prefab = GO.GetComponent<Prefab>();

		// idk cause if I add a prefab component when a new prefab is made, it enters here straight
		// so this check is to stop that??
		if (prefab.prefabGUID == (GUID)0)
		{
			return;
		}

		// if the resource does not exist anymore
		if (!Core::GetInstance()->GetResourceManager()->CheckResource(prefab.prefabGUID))
		{
			// actually i dont think this will ever trigger cause its on enter
			if (mPrefabMap.find(prefab.prefabGUID) != mPrefabMap.end())
			{
				auto& vec = mPrefabMap[prefab.prefabGUID];

				// remove from the vector
				for (auto iter = vec.begin(); iter != vec.end(); ++iter)
				{
					if (*iter == entity)
					{
						// only remove if its already inside
						GO.RemoveComponent<Prefab>();
						vec.erase(iter);
						break;
					}
				}
			}
		}
		// if the rsource exists, check if the entity is already in the map
		// cause if we're loading a new scene, this cehcks that
		else
		{
			// add it to prefab map
			mPrefabMap[prefab.prefabGUID].insert(entity);
		}
	}

	void PrefabSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	{

	}

	void PrefabSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{
		// TODO: Check if a prefab resource has been modified
		// if it has then update the instances createdw ith it
	}

	GameObject PrefabSystem::CreatePrefab(const GUID& prefabGUID, bool isEditor)
	{
		//		mShader = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Shader>((GUID)12204516898033894501);

		if (isEditor)
		{
			if (mPrefabBaseEntities.find(prefabGUID) != mPrefabBaseEntities.end())
			{
				GameObject GO = FactoryInstance.GetGOByEntity(mPrefabBaseEntities[prefabGUID]);
				return GO;
			}
		}

		Handle<SliceEngineTypes::Prefab> prefab = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Prefab>(prefabGUID);

		std::string name = prefab.get()->filePath;
		//Assets/GameObject_1.prefab
		Entity prefabEntity = JSONSerializer::DeserializePrefab(prefab.get()->filePath, isEditor);

		GameObject GO = FactoryInstance.GetGOByEntity(prefabEntity);
		//std::string goName = GO.GetName(); was for 
		if(!isEditor)
		{
			FactoryInstance.SetParent(GO.GetEntity()); // parent to scene?? idk
			mPrefabMap[prefabGUID].insert(GO.GetEntity());
		}

		//if (isEditor)
		//{
		// do this in DeserializePrefab instead
		//	mRegistry->emplace<PrefabEditingEntity>(prefabEntity);
		//}

		GO.AddComponent<Prefab>();
		GO.GetComponent<Prefab>().prefabGUID = prefabGUID;
		GO.GetComponent<Prefab>().prefabHandle = prefab;
		// add the children as well
		if (GO.HasComponent<SceneGraph>())
		{
			auto& sceneGraph = GO.GetComponent<SceneGraph>();
			Entity childEntity = sceneGraph.neighbours[SceneGraph::DOWN];
			while (childEntity != entt::null)
			{
				GameObject childGO = FactoryInstance.GetGOByEntity(childEntity);
				UpdatePrefabChild(childEntity, prefabGUID);
				auto& childSceneGraph = childGO.GetComponent<SceneGraph>();
				childEntity = childSceneGraph.neighbours[SceneGraph::RIGHT];
			}
		}

		// if its editor, then store the root entity
		if (isEditor)
		{
			mPrefabBaseEntities[prefabGUID] = GO.GetEntity();
		}

		return GO;
	}

	void PrefabSystem::UpdatePrefabChild(Entity entity, GUID const& guid, bool isEditor)
	{
		Handle<SliceEngineTypes::Prefab> prefab = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Prefab>(guid);
		GameObject GO = FactoryInstance.GetGOByEntity(entity);
		GO.AddComponent<Prefab>();
		GO.GetComponent<Prefab>().prefabGUID = guid;
		GO.GetComponent<Prefab>().prefabHandle = prefab;

		if (!isEditor)
		{
			mPrefabMap[guid].insert(GO.GetEntity());
		}

		if (isEditor)
		{
			mRegistry->emplace<PrefabEditingEntity>(entity);
		}
		auto& sceneGraph = GO.GetComponent<SceneGraph>();
		Entity childEntity = sceneGraph.neighbours[SceneGraph::DOWN];
		while (childEntity != entt::null)
		{
			GameObject childGO = FactoryInstance.GetGOByEntity(childEntity);
			UpdatePrefabChild(childEntity, guid);
			auto& childSceneGraph = childGO.GetComponent<SceneGraph>();
			childEntity = childSceneGraph.neighbours[SceneGraph::RIGHT];
		}
	}

	void PrefabSystem::InitEvent()
	{
		auto* eventManager = EventManager::GetInstance();

		eventManager->Subscribe<OnPrefabModifiedEvent, &PrefabSystem::OnPrefabModified>(this);
		eventManager->Subscribe<OnPrefabDeletedEvent, &PrefabSystem::OnPrefabDeleted>(this);
	}

	void PrefabSystem::OnPrefabModified(const OnPrefabModifiedEvent& event)
	{
		for (auto& [guid, vec] : mPrefabMap)
		{
			if (guid == event.guid)
			{
				// get a handle to the prefab so we can load data from it
				Handle<SliceEngineTypes::Prefab> prefab = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Prefab>(event.guid);
				std::string name = prefab.get()->filePath;
				//Assets/GameObject_1.prefab
				std::unordered_map<unsigned int, std::vector<rttr::variant>> prefabComponents = JSONSerializer::DeserializePrefabComponents(prefab.get()->filePath);

				// iterate through the entities that are made from this prefab
				for (auto entity : vec)
				{
					GameObject GO = FactoryInstance.GetGOByEntity(entity);

					auto& prefabComponent = GO.GetComponent<Prefab>();
					bool prefabIDFound = false;
					// Note: maybe check if the component data is the same? before replacing
					// also need to check if a component was deleted from the prefab then it should reflect
					// and also maybe not all components should be replaced? like transform should be left alone
					// get the prefab and it's data
					for (auto& [prefabID, compVar] :prefabComponents)
					{
						// if this GO has the same prefabID 
						// cause each obj of a prefab has a unique ID
						// since we need to know a relationship to which prefab obj is to which game object
						// when theres more than 1 object in a prefab
						if (prefabComponent.prefabID == prefabID)
						{
							prefabIDFound = true;
							// then emplace teh components of that entity
							for (auto& comp : compVar)
							{
								FactoryInstance.EmplaceComponents(entity, comp);

								rttr::type type = comp.get_type();
								if (type.is_wrapper())
								{
									type = type.get_wrapped_type();
								}

								std::string typeName = type.get_name().to_string();

								// reload all the handles here idfk how else to do it tbh
								// emplacing components will replace handles 
								// the other way is to retrieve specific variables only
								if (type.get_raw_type() == rttr::type::get<Renderer>())
								{
									auto& renderer = GO.GetComponent<Renderer>();
									renderer.materialHandle = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Material>(renderer.materialHandle.getGUID());
									renderer.modelHandle = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>(renderer.modelHandle.getGUID());
								}
								else if (type.get_raw_type() == rttr::type::get<Script>())
								{
									gScriptSystem->UpdateScriptVariables(entity);
									//auto& script = GO.GetComponent<Script>();
									////Script scriptComp = comp.get_value<Script>();
									//for(auto & [fieldName, fieldValue] : script.scriptableFieldMap)
									//{
									//	SLICE_LOG_DEBUG(fieldName);
									//	// set the field value to the component's field map
									//	//
									//	//script.scriptableFieldMap[fieldName] = fieldValue;
									//}
								}
								else if (comp.is_type<Animator>())
								{
									auto& animator = GO.GetComponent<Animator>();
									animator.Handle_stateMachine = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::StateMachine>(animator.Handle_stateMachine.getGUID());
									if (!animator.Handle_stateMachine.IsValid())
										animator.Handle_stateMachine = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::StateMachine>((GUID)9857886709116471337);
									animator.Handle_curr_anim_pkg = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::AnimationPackage>(animator.Handle_curr_anim_pkg.getGUID());
									animator.Handle_skeleton = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Skeleton>(animator.Handle_skeleton.getGUID());

									if (animator.Handle_stateMachine.IsValid())
									{
										animator.stateMachine.EFSM = *animator.Handle_stateMachine.get();
										animator.stateMachine.InitState();
									}

									if (animator.IsValid())
									{
										animator.curr_anim_pkg = *animator.Handle_curr_anim_pkg.get();
										animator.stateMachine.InitState(animator.curr_anim_pkg);
									}
								}
							}
						}
						//FactoryInstance.EmplaceComponents(entity, compVar);
					}

					// 
					if (prefabIDFound == false)
					{
						// this 
					}
				}
			}
		}
	}

	void PrefabSystem::OnPrefabDeleted(const OnPrefabDeletedEvent& event)
	{
		for (auto& [guid, vec] : mPrefabMap)
		{
			if (guid == event.guid)
			{
				// iterate through the entities that are made from this prefab
				// and remove the prefab component from them
				for (auto entity : vec)
				{
					// idk yet tbh
					auto GO = FactoryInstance.GetGOByEntity(entity);
					if (GO.HasComponent<Prefab>())
					{
						GO.RemoveComponent<Prefab>();
					}
				}

				// then erase this prefab from the map
				mPrefabMap.erase(event.guid);
				break;
			}
		}

	}
	
	void PrefabSystem::MakePrefab(Entity entity)
	{

		GameObject GO = FactoryInstance.GetGOByEntity(entity);
		GO.AddComponent<Prefab>();
		// maybe we can just use the entity id as a prefab id
		// i dont think prefab IDs have to be unique across prefabs??
		GO.GetComponent<Prefab>().prefabID = (unsigned int)entity;
		// add the children as well
		if (GO.HasComponent<SceneGraph>())
		{
			auto& sceneGraph = GO.GetComponent<SceneGraph>();
			Entity childEntity = sceneGraph.neighbours[SceneGraph::DOWN];
			while (childEntity != entt::null)
			{
				GameObject childGO = FactoryInstance.GetGOByEntity(childEntity);
				MakePrefab(childEntity);

				auto& childSceneGraph = childGO.GetComponent<SceneGraph>();
				childEntity = childSceneGraph.neighbours[SceneGraph::RIGHT];
			}
		}
	}

	/// <summary>
	/// Only used when a new prefab is created, and we have to update the existing entity in the scene
	/// </summary>
	/// <param name="entity">entity that we created prefab from</param>
	/// <param name="guid">GUID of the prefab</param>
	void PrefabSystem::UpdatePrefabComponent(Entity entity, GUID guid)
	{
		GameObject GO = FactoryInstance.GetGOByEntity(entity);
		Handle<SliceEngineTypes::Prefab> prefab = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Prefab>(guid);
		GO.GetComponent<Prefab>().prefabHandle = prefab;
		GO.GetComponent<Prefab>().prefabGUID = guid;
		mPrefabMap[guid].insert(entity);
		// add the children as well
		if (GO.HasComponent<SceneGraph>())
		{
			auto& sceneGraph = GO.GetComponent<SceneGraph>();
			Entity childEntity = sceneGraph.neighbours[SceneGraph::DOWN];
			while (childEntity != entt::null)
			{
				GameObject childGO = FactoryInstance.GetGOByEntity(childEntity);
				UpdatePrefabComponent(childEntity, guid);

				auto& childSceneGraph = childGO.GetComponent<SceneGraph>();
				childEntity = childSceneGraph.neighbours[SceneGraph::RIGHT];
			}
		}

	}

	void PrefabSystem::AddToPrefab(Entity entity, Entity rootNode)
	{
		GameObject rootGO = FactoryInstance.GetGOByEntity(rootNode);

		if (rootGO.HasComponent<Prefab>())
		{
			// same as the root GO
			GameObject GO = FactoryInstance.GetGOByEntity(entity);
			GO.AddComponent<Prefab>();
			GO.GetComponent<Prefab>().prefabGUID = rootGO.GetComponent<Prefab>().prefabGUID;
			GO.GetComponent<Prefab>().prefabHandle = rootGO.GetComponent<Prefab>().prefabHandle;
		}
	}

	bool PrefabSystem::IsNewGO(Entity entity, unsigned int prefabID)
	{
		return false;
	}
}