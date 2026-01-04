#include <pch.h>
#include "PrefabSystem.h"
#include "Core/Core.h"
#include "Serializer/JSONSerializer.h"
#include "Scripting/ScriptSystem.h"
#include "../Physics/PhysicsSystem.h"
#include "TransformSystem.h"

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
		if (prefab.prefabGUID == (GUID)0 || prefab.prefabGUID == GUID::null())
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
		//for (auto& [guid, ent] : mPrefabBaseEntities)
		//{
		//	if (ent == entity)
		//	{
		//		mPrefabBaseEntities.erase(guid);
		//		break;
		//	}
		//}

		auto& prefab = reg.get<Prefab>(entity);

		if (mPrefabMap.find(prefab.prefabGUID) != mPrefabMap.end())
		{
			auto& vec = mPrefabMap[prefab.prefabGUID];
			// remove from the vector
			for (auto iter = vec.begin(); iter != vec.end(); ++iter)
			{
				if (*iter == entity)
				{
					vec.erase(iter);
					break;
				}
			}
		}
	}

	void PrefabSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{
		// TODO: Check if a prefab resource has been modified
		// if it has then update the instances createdw ith it
	}

	//Creates a Prefab Game Object
	GameObject PrefabSystem::CreatePrefab(const GUID& prefabGUID, bool isEditor)
	{
		//		mShader = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Shader>((GUID)12204516898033894501);

		if (isEditor)
		{
			/*if (mPrefabBaseEntities.find(prefabGUID) != mPrefabBaseEntities.end())
			{
				GameObject GO = FactoryInstance.GetGOByEntity(mPrefabBaseEntities[prefabGUID]);
				return GO;
			}*/

			if (mPrefabEditable.second != entt::null || mPrefabEditable.first != GUID::null())
			{
				// if there is already a prefab being editable
				// then destroy it before opening a new one

				GameObject GO = FactoryInstance.GetGOByEntity(mPrefabEditable.second);
				GO.Destroy(); // destroy it
				mPrefabEditable.first = GUID::null();
				mPrefabEditable.second = entt::null;
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

		if (isEditor)
		{
			//do this in DeserializePrefab instead
			mRegistry->emplace_or_replace<PrefabEditingEntity>(prefabEntity);
		}

		if (!GO.HasComponent<Prefab>())
		{
			GO.AddComponent<Prefab>();
		}
		//
		GO.GetComponent<Prefab>().prefabGUID = prefabGUID;
		GO.GetComponent<Prefab>().prefabHandle = prefab;
		mNextPrefabID[prefabGUID] = GO.GetComponent<Prefab>().prefabID;
		// add the children as well
		if (GO.HasComponent<SceneGraph>())
		{
			auto& sceneGraph = GO.GetComponent<SceneGraph>();
			Entity childEntity = sceneGraph.neighbours[SceneGraph::DOWN];
			while (childEntity != entt::null)
			{
				GameObject childGO = FactoryInstance.GetGOByEntity(childEntity);
				UpdatePrefabChild(childEntity, prefabGUID, isEditor);
				auto& childSceneGraph = childGO.GetComponent<SceneGraph>();
				childEntity = childSceneGraph.neighbours[SceneGraph::RIGHT];
			}
		}

		// if its editor, then store the root entity
		if (isEditor)
		{
			//mPrefabBaseEntities[prefabGUID] = GO.GetEntity();

			mPrefabEditable.first = prefabGUID;
			mPrefabEditable.second = prefabEntity;
		}

		return GO;
	}

	void PrefabSystem::UpdatePrefabChild(Entity entity, GUID const& guid, bool isEditor)
	{
		Handle<SliceEngineTypes::Prefab> prefab = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Prefab>(guid);
		GameObject GO = FactoryInstance.GetGOByEntity(entity);
		//GO.AddComponent<Prefab>();
		GO.GetComponent<Prefab>().prefabGUID = guid;
		GO.GetComponent<Prefab>().prefabHandle = prefab;

		// get the biggest id to start from
		if (GO.GetComponent<Prefab>().prefabID > mNextPrefabID[guid])
		{
			mNextPrefabID[guid] = GO.GetComponent<Prefab>().prefabID;
		}
		//if (!isEditor)
		//{
		// only the root entity should get added to the map
		//	mPrefabMap[guid].insert(GO.GetEntity());
		//}

		if (isEditor)
		{
			mRegistry->emplace_or_replace<PrefabEditingEntity>(entity);
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
		eventManager->Subscribe<OnPrefabSerializedEvent, &PrefabSystem::OnPrefabSerialized>(this);
	}

	void PrefabSystem::OnPrefabModified(const OnPrefabModifiedEvent& event)
	{
		// get all the entities in the prefab that was modified
		std::vector<Entity> prefabEntities;
		std::unordered_map<unsigned int, Entity> originalPrefabIDToEntityMap;

		prefabEntities.push_back(event.entity);
		GameObject GO = FactoryInstance.GetGOByEntity(event.entity);

		if (GO.HasComponent<SceneGraph>())
		{
			auto& sceneGraph = GO.GetComponent<SceneGraph>();
			Entity childEntity = sceneGraph.neighbours[SceneGraph::DOWN];
			while (childEntity != entt::null)
			{
				GameObject childGO = FactoryInstance.GetGOByEntity(childEntity);
				GetEntityFromPrefab(prefabEntities, originalPrefabIDToEntityMap, childGO.GetEntity());
				auto& childSceneGraph = childGO.GetComponent<SceneGraph>();
				childEntity = childSceneGraph.neighbours[SceneGraph::RIGHT];
			}
		}

		// the prefab doesn't exist in the perfab map
		if (!mPrefabMap.contains(event.guid))
			return;

		for (auto entity : mPrefabMap[event.guid])
		{
			std::vector<Entity> prefabInstanceEntities;
			std::unordered_map<unsigned int, Entity> instancePrefabIDToEntityMap;

			// get all the entities and the children entities of an instance of the prefab
			GameObject GOInstance = FactoryInstance.GetGOByEntity(entity);
			if (GOInstance.HasComponent<Prefab>())
			{
				instancePrefabIDToEntityMap[GOInstance.GetComponent<Prefab>().prefabID] = entity;
			}

			prefabInstanceEntities.push_back(entity);
			if (GOInstance.HasComponent<SceneGraph>())
			{
				auto& sceneGraph = GOInstance.GetComponent<SceneGraph>();
				Entity childEntity = sceneGraph.neighbours[SceneGraph::DOWN];
				while (childEntity != entt::null)
				{
					GameObject childGO = FactoryInstance.GetGOByEntity(childEntity);
					GetEntityFromPrefab(prefabInstanceEntities, instancePrefabIDToEntityMap, childGO.GetEntity());
					auto& childSceneGraph = childGO.GetComponent<SceneGraph>();
					childEntity = childSceneGraph.neighbours[SceneGraph::RIGHT];
				}
			}

			// now that we have both the vector of entities from the prefab
			// and the vector of entities from the instance
			// and maps of prefab id to entity
			// we can compare the two and update it

			// first handle new game objects
			if (prefabEntities.size() > prefabInstanceEntities.size())
			{
				for (auto& [prefabID, entity] : originalPrefabIDToEntityMap)
				{
					// if the instance doesnt have that prefab id means its a new one
					if (!instancePrefabIDToEntityMap.contains(prefabID))
					{

					}
				}
			}
			
			for (size_t index = 0; index < prefabEntities.size(); ++index)
			{

			}
		}

	}

	void PrefabSystem::GetEntityFromPrefab(std::vector<Entity>& prefabEntities, std::unordered_map<unsigned int, Entity>& prefabToEntity, Entity entity)
	{
		GameObject GO = FactoryInstance.GetGOByEntity(entity);
		prefabEntities.push_back(entity);
		if (GO.HasComponent<Prefab>())
		{
			prefabToEntity[GO.GetComponent<Prefab>().prefabID] = entity;
		}

		if (GO.HasComponent<SceneGraph>())
		{
			auto& sceneGraph = GO.GetComponent<SceneGraph>();
			Entity childEntity = sceneGraph.neighbours[SceneGraph::DOWN];
			while (childEntity != entt::null)
			{
				GameObject childGO = FactoryInstance.GetGOByEntity(childEntity);
				GetEntityFromPrefab(prefabEntities, prefabToEntity, childGO.GetEntity());
				auto& childSceneGraph = childGO.GetComponent<SceneGraph>();
				childEntity = childSceneGraph.neighbours[SceneGraph::RIGHT];
			}
		}
	}

	void PrefabSystem::OnPrefabDeleted(const OnPrefabDeletedEvent& event)
	{

	}
	
	void PrefabSystem::OnPrefabSerialized(const OnPrefabSerializedEvent& event)
	{

	}

	void PrefabSystem::UpdateBasePrefabs()
	{
		if (mPrefabEditable.first == GUID::null() || mPrefabEditable.second == entt::null)
			return;

		auto& transformSys = Core::GetInstance()->GetSystem<TransformSystem>();
		// update all the base entities
		// their root is the base entity as only the base gets added to the map
	/*	for (auto& [guid, entity] : mPrefabBaseEntities)
		{
			transformSys.UpdateWorldMatrix(entity, glm::mat4(1.0f));
		}*/
		
		transformSys.UpdateWorldMatrix(mPrefabEditable.second, glm::mat4(1.0f));


	}

	/// <summary>
	/// Only ever called when making an object into a prefab for the first time
	/// Prefab IDs are just gonna be an incremental int value for each object in the prefab
	/// </summary>
	/// <param name="entity"></param>
	void PrefabSystem::MakePrefab(Entity entity)
	{
		unsigned int prefabID = 0;
		GameObject GO = FactoryInstance.GetGOByEntity(entity);

		GO.AddComponent<Prefab>();
		// maybe we can just use the entity id as a prefab id
		// i dont think prefab IDs have to be unique across prefabs??
		GO.GetComponent<Prefab>().prefabID = prefabID;
		prefabID++;
		// add the children as well
		if (GO.HasComponent<SceneGraph>())
		{
			auto& sceneGraph = GO.GetComponent<SceneGraph>();
			Entity childEntity = sceneGraph.neighbours[SceneGraph::DOWN];
			while (childEntity != entt::null)
			{
				GameObject childGO = FactoryInstance.GetGOByEntity(childEntity);
				MakePrefabChild(childEntity, prefabID);

				auto& childSceneGraph = childGO.GetComponent<SceneGraph>();
				childEntity = childSceneGraph.neighbours[SceneGraph::RIGHT];
			}
		}
	}

	void PrefabSystem::MakePrefabChild(Entity entity, unsigned int& prefabID)
	{
		GameObject GO = FactoryInstance.GetGOByEntity(entity);
		GO.AddComponent<Prefab>();
		// maybe we can just use the entity id as a prefab id
		// i dont think prefab IDs have to be unique across prefabs??
		GO.GetComponent<Prefab>().prefabID = prefabID;
		prefabID++;
		// add the children as well
		if (GO.HasComponent<SceneGraph>())
		{
			auto& sceneGraph = GO.GetComponent<SceneGraph>();
			Entity childEntity = sceneGraph.neighbours[SceneGraph::DOWN];
			while (childEntity != entt::null)
			{
				GameObject childGO = FactoryInstance.GetGOByEntity(childEntity);
				MakePrefabChild(childEntity, prefabID);

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
	void PrefabSystem::UpdatePrefabComponent(Entity entity, GUID guid, bool toRemove)
	{
		GameObject GO = FactoryInstance.GetGOByEntity(entity);

		//Prefab just being added
		if(!toRemove)
		{
			Handle<SliceEngineTypes::Prefab> prefab = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Prefab>(guid);
			GO.GetComponent<Prefab>().prefabHandle = prefab;
			GO.GetComponent<Prefab>().prefabGUID = guid;
			mPrefabMap[guid].insert(entity);
			//mPrefabIDs[guid].push_back(GO.GetComponent<Prefab>().prefabID);
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
		//Removing Prefab Component
		if (toRemove)
		{
			//Prefab GUID
			GUID prefabGUID = GO.GetComponent<Prefab>().prefabGUID;
			//Remove entity from the set
			if (mPrefabMap[guid].contains(entity))
				mPrefabMap[guid].erase(entity);

			//Remove Recursively
			if (GO.HasComponent<SceneGraph>())
			{
				auto& sceneGraph = GO.GetComponent<SceneGraph>();
				Entity childEntity = sceneGraph.neighbours[SceneGraph::DOWN];
				while (childEntity != entt::null)
				{
					GameObject childGO = FactoryInstance.GetGOByEntity(childEntity);
					UpdatePrefabComponent(childEntity, guid, true);

					auto& childSceneGraph = childGO.GetComponent<SceneGraph>();
					childEntity = childSceneGraph.neighbours[SceneGraph::RIGHT];
				}
			}

			//Remove Component should be the last thing
			GO.RemoveComponent<Prefab>();
		}

	}

	void PrefabSystem::AddToPrefab(Entity entity, Entity rootNode)
	{
		GameObject rootGO = FactoryInstance.GetGOByEntity(rootNode);

		if (rootGO.HasComponent<Prefab>())
		{
			mNextPrefabID[rootGO.GetComponent<Prefab>().prefabGUID]++;
			unsigned int prefabID = mNextPrefabID[rootGO.GetComponent<Prefab>().prefabGUID];
			// same as the root GO
			GameObject GO = FactoryInstance.GetGOByEntity(entity);
			GO.AddComponent<Prefab>();
			GO.GetComponent<Prefab>().prefabGUID = rootGO.GetComponent<Prefab>().prefabGUID;
			GO.GetComponent<Prefab>().prefabHandle = rootGO.GetComponent<Prefab>().prefabHandle;
			GO.GetComponent<Prefab>().prefabID = prefabID;
			GO.AddComponent<PrefabEditingEntity>();
		}
	}

	bool PrefabSystem::IsNewGO(Entity entity, unsigned int prefabID)
	{
		return false;
	}
}