#include <pch.h>
#include "PrefabSystem.h"
#include "Core/Core.h"
#include "Serializer/JSONSerializer.h"

namespace SliceEngine
{
	void PrefabSystem::EntityOnEnter(entt::registry& reg, entt::entity entity)
	{

	}

	void PrefabSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	{

	}

	void PrefabSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{

	}

	GameObject PrefabSystem::CreatePrefab(const GUID& prefabGUID)
	{
		//		mShader = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Shader>((GUID)12204516898033894501);

		Handle<SliceEngineTypes::Prefab> prefab = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Prefab>(prefabGUID);
		
		Entity prefabEntity = JSONSerializer::DeserializePrefab(prefab.get()->filePath);
		GameObject GO = FactoryInstance.GetGOByEntity(prefabEntity);
		FactoryInstance.SetParent(GO.GetEntity()); // parent to scene?? idk

		mPrefabMap[prefabGUID].push_back(GO.GetEntity());

		GO.AddComponent<Prefab>();
		GO.GetComponent<Prefab>().prefabGUID = prefabGUID;
		GO.GetComponent<Prefab>().prefabHandle = prefab;

		return GO;
	}
}