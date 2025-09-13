#ifndef GO_FACTORY_H
#define GO_FACTORY_H

#include "GameObject.h"
#include <entt.hpp>

namespace SliceEngine
{
	using ComponentCloner = std::function<void(Registry& reg, Entity eToClone, Entity eToCreate)>;

	struct SliceEntity {};

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
					/*if (auto* component = reg.try_get<T>(toClone))
					{
						if constexpr (std::is_empty_v<T>)
						{
							reg.emplace_or_replace<T>(d);
						}
						else
						{
							reg.emplace_or_replace<T>(d, *c);
						}
					}*/
				});
		};

		GameObject& CreateGO(std::string name = "GameObject");
		GameObject& CreateUIGO(std::string name = "UI_GameObject");
		GameObject CloneGO(GameObject & go);
		void Destroy(Entity const& entity);
		void Destroy(GameObject& go);
		void TestLoop();
		void UpdateDestroyed();
		std::string CreateName(std::string name);

		Registry* mRegistry;
	private:
		std::unordered_map<std::string, Entity> mNameToEntity;
		std::unordered_map<Entity, GameObject> mEntityToGO;
		std::unordered_map<entt::id_type, ComponentCloner> mComponentCloners;

		std::set<Entity> mDeleteList;


	};
}

#endif