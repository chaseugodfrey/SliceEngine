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

	void GameObject::SetName(std::string name)
	{
		if (HasComponent<SliceEntity>())
		{
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