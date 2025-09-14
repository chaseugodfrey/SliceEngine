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

		GameObject() = default;

		GameObject(Registry& reg, Entity entity);

		~GameObject()
		{

		};

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{			
			if (!IsValid())
			{
				assert("why the fk");
			}
			mRegistry->emplace_or_replace<T>(mEntity, std::forward<Args>(args)...);
			return GetComponent<T>();
		}

		template<typename T>
		void RemoveComponent()
		{
			mRegistry->remove<T>(mEntity);
		}

		template<typename T>
		bool HasComponent()
		{
			mRegistry.all_of<T>(mEntity);
		}

		bool IsValid() const
		{
			return mRegistry && mRegistry->valid(mEntity);
		}

		template<typename T>
		T& GetComponent()
		{
			T* component = mRegistry->try_get<T>(mEntity);

			if (component)
			{
				return *component;
			}

			T temp{};
			// NOTE: Throw error when component does not exist
			return temp;
		}

		void SetName(std::string name);

		std::string GetName();

		void Destroy();

		Entity GetEntity() const;

	private:
		Entity mEntity{entt::null};
		std::string mName{};
		Registry* mRegistry{ nullptr };

	};

}


#endif