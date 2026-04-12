/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			GameObject.h
 author:		Gideon Francis
 email:			g.francis@digipen.edu
 brief:			Wrapper for entities

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "ECSTypes.h"
#include <entt.hpp>
//q#include "GOFactory.h"

namespace SliceEngine
{
	class GameObject
	{
	public:
		friend class GOFactory;

		GameObject();

		GameObject(Registry& reg, Entity entity);



		~GameObject()
		{

		};

		template<typename T, typename... Args>
		void AddComponent(Args&&... args)
		{
			if (!IsValid())
			{
				assert("why the fk");
			}

			if (HasComponent<T>())
			{
				return;
			}

			mRegistry.emplace_or_replace<T>(mEntity, std::forward<Args>(args)...);
			//return GetComponent<T>();
		}

		template<typename T>
		void RemoveComponent()
		{
			if (!IsValid())
			{
				assert("why the fk");
			}

			if (!HasComponent<T>())
			{
				return;
			}

			mRegistry.remove<T>(mEntity);
		}

		template<typename T>
		bool HasComponent() const
		{
			return mRegistry.all_of<T>(mEntity);
		}

		bool IsValid() const;

		template<typename T>
		T& GetComponent()
		{
			T* component = mRegistry.try_get<T>(mEntity);

			if (component)
			{
				return *component;
			}
			else
			{
				//TODO: Change to actual error log and assert
				// but we can also just assert ourselves i guess
				assert("why the fk");
			}

			// in debug, EnTT will assert if it doesn't exist
			return mRegistry.get<T>(mEntity);
		}

		template<typename T>
		const T& GetComponent() const
		{
			T* component = mRegistry.try_get<T>(mEntity);

			if (component)
			{
				return *component;
			}
			else
			{
				//TODO: Change to actual error log and assert
				// but we can also just assert ourselves i guess
				assert("why the fk");
			}

			// in debug, EnTT will assert if it doesn't exist
			return mRegistry.get<T>(mEntity);
		}

		bool operator<(const GameObject& other) const {
			return mEntity < other.mEntity; 
		}

		bool operator==(const GameObject& other) const
		{
			return mEntity == other.mEntity && &mRegistry == &other.mRegistry;
		}

		std::vector<Entity> GetAllChildren();

		GameObject& operator=(const GameObject& other)  
		{
			if(this != &other)
			{
				mEntity = other.mEntity;
			}
			return *this;
		}

		void SetName(std::string name);

		std::string GetName();
		const std::string GetName() const;

		void SetTag(std::string const& tag);
		std::string GetTag();
		const std::string GetTag() const;

		void Destroy();

		Entity GetEntity() const;

	private:
		Entity mEntity{entt::null};
		//std::string mName{};
		Registry& mRegistry;

	};

}


#endif