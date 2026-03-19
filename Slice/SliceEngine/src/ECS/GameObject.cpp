/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			GameObject.cpp
 author:		Gideon Francis
 email:			g.francis@digipen.edu
 brief:			Wrapper for entities

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "GameObject.h"
#include <utility>
#include "Core/Core.h"

namespace SliceEngine
{
	GameObject::GameObject() : mRegistry(Core::GetInstance()->mFactory.mRegistry) {}

	GameObject::GameObject(Registry& reg, Entity entity) : mRegistry(reg), mEntity(entity)
	{
		//mEntity = mRegistry->create();
	}

	std::vector<Entity> GameObject::GetAllChildren()
	{

		if (HasComponent<SceneGraph>())
		{
			std::vector<Entity> children;
			SceneGraph& sceneGraph = GetComponent<SceneGraph>();
			Entity childEntity = sceneGraph.neighbours[SceneGraph::DOWN];
			while (childEntity != entt::null)
			{
				children.push_back(childEntity);

				GameObject childGO = FactoryInstance.GetGOByEntity(childEntity);
				std::vector<Entity> childChildren = childGO.GetAllChildren();

				children.insert(children.end(), childChildren.begin(), childChildren.end());
				SceneGraph& childSceneGraph = mRegistry.get<SceneGraph>(childEntity);
				childEntity = childSceneGraph.neighbours[SceneGraph::RIGHT];
			}
			return children;
		}
		return std::vector<Entity>();
	}

	void GameObject::SetName(std::string name)
	{
		if (HasComponent<SliceEntity>())
		{
			// if the current GO in the map is the same as the current entity and the name is the same
			// then dont do anything
			if (FactoryInstance.GetGOByName(name).mEntity == mEntity)
			{
				return;
			}

			std::string newName = FactoryInstance.CreateName(name);
			FactoryInstance.UpdateName(newName, mEntity);
			GetComponent<SliceEntity>().mName = newName;
		}
	}

	std::string GameObject::GetName()
	{
		if (HasComponent<SliceEntity>())
		{
			return GetComponent<SliceEntity>().mName;
		}
		return std::string(); // blank str
	}

	const std::string GameObject::GetName() const
	{
		if (HasComponent<SliceEntity>())
		{
			return GetComponent<SliceEntity>().mName;
		}
		return std::string(); // blank str
	}

	void GameObject::SetTag(std::string const& tag)
	{
		if (HasComponent<SliceEntity>())
		{
			GetComponent<SliceEntity>().mTag = tag;
		}
	}

	std::string GameObject::GetTag()
	{
		if (HasComponent<SliceEntity>())
		{
			return GetComponent<SliceEntity>().mTag;
		}
		return std::string();
	}

	const std::string GameObject::GetTag() const
	{
		if (HasComponent<SliceEntity>())
		{
			return GetComponent<SliceEntity>().mTag;
		}
		return std::string();
	}

	void GameObject::Destroy()
	{
		////std::cout << "Destryoing entity in gameobject cpp: " << (uint32_t)mEntity << std::endl;
		if (mEntity == entt::null)
		{
			SLICE_LOG("entity is null???");
			return;
		}
		mRegistry.destroy(mEntity);

		//mRegistry.eac
	}

	 Entity GameObject::GetEntity() const
	{
		return mEntity;
	}

	 bool GameObject::IsValid() const
	 {
		 return mRegistry.valid(mEntity);
	 }

}