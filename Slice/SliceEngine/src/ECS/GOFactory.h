#ifndef GO_FACTORY_H
#define GO_FACTORY_H
#include "ECSTypes.h"
#include "GameObject.h"
#include <entt.hpp>

namespace SliceEngine
{
	using ComponentCloner = std::function<void(Registry& reg, Entity eToClone, Entity eToCreate)>;
	//using EnttIdToRttrType = std::function<rttr::type(entt::id_type type)>;
	//using GetterMapper = std::function<rttr::instance(entt::registry&, entt::entity)>;
	//using InstanceGetter = std::unordered_map<entt::id_type, GetterMapper>;

	//static EnttIdToRttrType EnttIdToRttrTypeFunc;
	//static InstanceGetter InstanceGetterFunc;

	class GOFactory
	{
	public:
		GOFactory();
		~GOFactory();

		// uses entt's emplace_or_replace to clone components
		// this uses component's copy/move constructor
		// so any components that has any pointers or handles, will need a custom clone function
		// but I'm not doing that now
		// if yall need it then lmk
		template<class T>
		void CreateComponentCloner()
		{
			const entt::id_type id = entt::type_id<T>().hash();
			mComponentCloners.emplace(id, [](Registry& reg, Entity toClone, Entity toCreate)
				{
					if (auto* component = reg.try_get<T>(toClone))
					{
						if constexpr (std::is_empty_v<T>)
						{
							reg.emplace_or_replace<T>(toCreate);
						}
						else
						{
							reg.emplace_or_replace<T>(toCreate, *component);
						}
					}	
				});
		};

		//template<class T>
		//void MapEnttToRTTR()
		//{
		//	auto id = entt::type_id<T>().hash();

		//	// Store RTTR type mapping
		//	mEnttTypeIdToRttrType[id] = rttr::type::get<T>();

		//	// Store instance getter
		//	InstanceGetterFunc[id] = [](entt::registry& reg, entt::entity e) -> rttr::instance
		//		{
		//			if (auto* comp = reg.try_get<T>(e))
		//				return rttr::instance(*comp);

		//			std::cerr << "[RTTR] Component not found for entity\n";
		//			return rttr::instance(); // invalid
		//		};
		//}
		
		GameObject CreateEO();
		GameObject CreateGO(std::string name = "GameObject");
		GameObject CreateUIGO(std::string name = "UI_GameObject");
		GameObject CloneGO(GameObject const& go);
		void Destroy(GameObject& go);
		void TestLoop();
		void UpdateDestroyed();
		std::string CreateName(std::string name);

		Registry mRegistry;
	private:

		std::unordered_map<std::string, Entity> mNameToEntity;
		std::unordered_map<Entity, GameObject> mEntityToGO;		
		std::unordered_map<entt::id_type, ComponentCloner> mComponentCloners;
		std::vector<GameObject> mEngineEntities;
		std::set<Entity> mDeleteList;
		//std::unordered_map<entt::id_type, rttr::type> mEnttTypeIdToRttrType;
	};
}

#endif