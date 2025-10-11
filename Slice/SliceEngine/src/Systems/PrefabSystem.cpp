#include <pch.h>
#include "PrefabSystem.h"
#include "Core/Core.h"

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
		GameObject GO = FactoryInstance.CreateBlank();
		mPrefabToEntity[prefab] = GO.GetEntity();

		

		return FactoryInstance.CreateGO("temp");
	}
}