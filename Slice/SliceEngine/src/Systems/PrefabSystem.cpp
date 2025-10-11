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
		return FactoryInstance.CreateGO("temp");
	}
}