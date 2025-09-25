#include <pch.h>
#include "GameObject.h"
#include <utility>
#include "Core/Core.h"

namespace SliceEngine
{
	GameObject::GameObject() : mRegistry(Core::GetInstance()->mFactory.mRegistry) {}

	GameObject::GameObject(Registry& reg, Entity entity, std::string name) : mRegistry(reg), mEntity(entity), mName(name)
	{
		//mEntity = mRegistry->create();
	}

	void GameObject::SetName(std::string name)
	{

		//mName = name;
	}

	std::string GameObject::GetName()
	{
		return mName;
	}

	const std::string GameObject::GetName() const
	{
		return mName;
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