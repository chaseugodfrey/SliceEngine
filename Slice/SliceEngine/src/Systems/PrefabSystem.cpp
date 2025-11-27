#include <pch.h>
#include "PrefabSystem.h"
#include "Core/Core.h"
#include "Serializer/JSONSerializer.h"

namespace SliceEngine
{
	void PrefabSystem::EntityOnEnter(entt::registry& reg, entt::entity entity)
	{
		// TODO: When an entity comes into the system
		// check if the prefab it references exists/is modified
		// if not then unprefab it (remove prefab component)
	}

	void PrefabSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	{

	}

	void PrefabSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{
		// TODO: Check if a prefab resource has been modified
		// if it has then update the instances createdw ith it
	}

	GameObject PrefabSystem::CreatePrefab(const GUID& prefabGUID)
	{
		//		mShader = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Shader>((GUID)12204516898033894501);

		Handle<SliceEngineTypes::Prefab> prefab = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Prefab>(prefabGUID);

		std::string name = prefab.get()->filePath;
		//Assets/GameObject_1.prefab
		Entity prefabEntity = JSONSerializer::DeserializePrefab(prefab.get()->filePath);

		GameObject GO = FactoryInstance.GetGOByEntity(prefabEntity);
		//std::string goName = GO.GetName(); was for debugging
		FactoryInstance.SetParent(GO.GetEntity()); // parent to scene?? idk

		mPrefabMap[prefabGUID].push_back(GO.GetEntity());

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
				//childGO.AddComponent<Prefab>();
				//childGO.GetComponent<Prefab>().prefabGUID = prefabGUID;
				//childGO.GetComponent<Prefab>().prefabHandle = prefab;
				auto& childSceneGraph = childGO.GetComponent<SceneGraph>();
				childEntity = childSceneGraph.neighbours[SceneGraph::RIGHT];
			}
		}

		return GO;
	}

	void PrefabSystem::UpdatePrefabChild(Entity entity, GUID const& guid)
	{
		Handle<SliceEngineTypes::Prefab> prefab = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Prefab>(guid);
		GameObject GO = FactoryInstance.GetGOByEntity(entity);
		GO.AddComponent<Prefab>();
		GO.GetComponent<Prefab>().prefabGUID = guid;
		GO.GetComponent<Prefab>().prefabHandle = prefab;
		mPrefabMap[guid].push_back(GO.GetEntity());
		auto& sceneGraph = GO.GetComponent<SceneGraph>();
		Entity childEntity = sceneGraph.neighbours[SceneGraph::DOWN];
		while (childEntity != entt::null)
		{
			GameObject childGO = FactoryInstance.GetGOByEntity(childEntity);
			UpdatePrefabChild(childEntity, guid);
			//childGO.AddComponent<Prefab>();
			//childGO.GetComponent<Prefab>().prefabGUID = prefabGUID;
			//childGO.GetComponent<Prefab>().prefabHandle = prefab;
			auto& childSceneGraph = childGO.GetComponent<SceneGraph>();
			childEntity = childSceneGraph.neighbours[SceneGraph::RIGHT];
		}
	}
	void PrefabSystem::MakePrefab(Entity entity, GUID guid)
	{
		GameObject GO = FactoryInstance.GetGOByEntity(entity);
		Handle<SliceEngineTypes::Prefab> prefab = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Prefab>(guid);
		GO.AddComponent<Prefab>();
		GO.GetComponent<Prefab>().prefabGUID = guid;
		GO.GetComponent<Prefab>().prefabHandle = prefab;
		mPrefabMap[guid].push_back(GO.GetEntity());
		// add the children as well
		if (GO.HasComponent<SceneGraph>())
		{
			auto& sceneGraph = GO.GetComponent<SceneGraph>();
			Entity childEntity = sceneGraph.neighbours[SceneGraph::DOWN];
			while (childEntity != entt::null)
			{
				GameObject childGO = FactoryInstance.GetGOByEntity(childEntity);
				UpdatePrefabChild(childEntity, guid);
				//childGO.AddComponent<Prefab>();
				//childGO.GetComponent<Prefab>().prefabGUID = prefabGUID;
				//childGO.GetComponent<Prefab>().prefabHandle = prefab;
				auto& childSceneGraph = childGO.GetComponent<SceneGraph>();
				childEntity = childSceneGraph.neighbours[SceneGraph::RIGHT];
			}
		}
	}
}