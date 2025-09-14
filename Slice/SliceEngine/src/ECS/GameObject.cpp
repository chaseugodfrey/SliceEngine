#include <pch.h>
#include "GameObject.h"
#include <utility>

namespace SliceEngine
{
	GameObject::GameObject(Registry& reg, Entity entity) : mRegistry(&reg), mEntity(entity)
	{
		//mEntity = mRegistry->create();
	}

	void GameObject::SetName(std::string name)
	{
		mName = name;
	}

	std::string GameObject::GetName()
	{
		return mName;
	}

	void GameObject::Destroy()
	{
		mRegistry->destroy(mEntity);

		//mRegistry.eac
	}

	 Entity GameObject::GetEntity() const
	{
		return mEntity;
	}

	 bool GameObject::IsValid() const
	 {
		 return mRegistry && mRegistry->valid(mEntity);
	 }

}