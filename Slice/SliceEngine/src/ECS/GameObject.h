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

		GameObject(Registry& reg);

		~GameObject()
		{

		};

		template<typename T, typename... Args>
		void AddComponent(Args&&... args)
		{
			//rttr::type tType = rttr::type::get<T>();
			//rttr::type transformType = rttr::type::get<Transform>();
			//rttr::type uiTransformType = rttr::type::get<UITransform>();

			//// If adding transform or UI Transform
			//// check to make sure it doesn't already have one
			//// if it does then dont let it add
			//// changing between UI and Transform should be done in another function
			//if (tType == transformType || tType == uiTransformType)
			//{
			//	if (mRegistry->any_of<Transform, UITransform>(mEntity))
			//	{
			//		if (tType == uiTransformType)
			//		{
			//			// log an error saying cant add UI transform to transform

			//			return;
			//		}
			//	}
			//}

			// Get UI transform here once we have it
			// then check if it already has transform dont add uitransform and vice versa
			
			mRegistry->emplace<T>(mEntity, std::forward<Args>(args)...);

			// idk why EnTT can't return a get<T>
			//return mRegistry->get<T>(mEntity);
		}

		template<typename T>
		void ChangeTransform()
		{
			//rttr::type tType = rttr::type::get<T>();
			//rttr::type transformType = rttr::type::get<Transform>();
			//rttr::type uiTransformType = rttr::type::get<UITransform>();

			//if (tType == transformType)
			//{
			//	// if it has a UI transform changing to transform
			//	if (mRegistry->any_of<UITransform>(mEntity))
			//	{
			//		// idk what to do yet when we wanna do this
			//		// maybe clear all non compatible components?
			//	}
			//}
			//else if (tType == uiTransformType)
			//{
			//	// if it has transform changing to UI transform
			//	if (mRegistry->any_of<Transform>(mEntity))
			//	{
			//		// idk what to do yet when we wanna do this
			//		// maybe clear all non compatible components?
			//	}
			//}
		}

		template<typename T>
		void RemoveComponent()
		{
			//rttr::type tType = rttr::type::get<T>();

			//if (tType == rttr::type::get<Transform>())
			//{
			//	// TODO_Gideon: log an error saying can't remove transform or smth

			//	return;
			//}

			mRegistry->remove<T>(mEntity);
		}

		template<typename T>
		bool HasComponent()
		{
			mRegistry.all_of<T>(mEntity);
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

		Entity GetEntity();

	private:
		Entity mEntity{};
		std::string mName{};
		Registry* mRegistry{ nullptr };

	};

}


#endif