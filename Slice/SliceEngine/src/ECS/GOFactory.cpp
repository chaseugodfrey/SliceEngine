#include <pch.h>

#include "GOFactory.h"


namespace SliceEngine
{
	GOFactory::GOFactory()
	{

	}

	GOFactory::~GOFactory()
	{

	}

	GameObject GOFactory::CreateEO()
	{
		Entity entity = mRegistry.create();
		GameObject go(mRegistry, entity);
		// Don't add to map because its not a game object
		go.AddComponent<Transform>();
		go.AddComponent<EngineEntity>();
		mEngineEntities.emplace_back(go);

		return go;
	}

	GameObject GOFactory::CreateGO(std::string name)
	{
		//Entity go = mRegistry.create();
		Entity entity = mRegistry.create();
		GameObject go(mRegistry, entity);

		go.SetName(CreateName(name));
		mNameToEntity.insert(std::make_pair(go.GetName(), go.GetEntity()));
		mEntityToGO.insert(std::make_pair(go.GetEntity(), go));

		// Can add default components here like transform
		//mRegistry.emplace_or_replace<Transform>(go);
		go.AddComponent<Transform>();
		// Every entity created will keep this flag for easy pulling
		go.AddComponent<SliceEntity>();

		return go;
	}

	GameObject GOFactory::CreateUIGO(std::string name)
	{
		//Entity go = mRegistry.create();

		Entity entity = mRegistry.create();
		GameObject go(mRegistry, entity);
		go.SetName(CreateName(name));
		mNameToEntity.insert(std::make_pair(go.GetName(), go.GetEntity()));
		mEntityToGO.insert(std::make_pair(go.GetEntity(), go));

		// Can add default components here like UITransform

		// Every entity created will keep this flag for easy pulling
		//mRegistry.emplace<SliceEntity>(go);
		go.AddComponent<SliceEntity>();

		return go;

	}

	GameObject GOFactory::CloneGO(GameObject const& go)
	{
		Entity entity = mRegistry.create();
		GameObject newGO(mRegistry, entity);

		newGO.SetName(CreateName(newGO.GetName()));

		// loop through every component cloner to clone the component onto the new entity
		for (auto& cloner : mComponentCloners)
		{
			cloner.second(mRegistry, go.GetEntity(), newGO.GetEntity());
		}

		////mNameToEntity.insert(std::make_pair("Test", newGO));
		////mEntityToGO.insert(std::make_pair(newGO.GetEntity(), newGO));

		return newGO;
	}

	void GOFactory::Destroy(GameObject& go)
	{
		mDeleteList.insert(go.GetEntity());
	}

	void GOFactory::TestLoop()
	{
		auto entityView = mRegistry.view<SliceEntity>();
		for (auto entity : entityView)
		{
			std::cout << mEntityToGO[entity].GetName() << std::endl;
			
			
			for (auto&& [type_id, storage] : mRegistry.storage())
			{
				if (storage.contains(entity))
				{
					// each component will be here
					std::cout << storage.type().name() << std::endl;
				}
			}
			//auto type = Registry::visi
		}
	}

	/// <summary>
	/// Call this at the end of update loop
	/// so that entity deletion is done after one complete iteration
	/// </summary>
	void GOFactory::UpdateDestroyed()
	{
		for (auto& Entity : mDeleteList)
		{
			// idk if its okay to destroy EnTT entity before clearing from map
			// but ill leave it like this for now

			// erase from the maps
			mNameToEntity.erase(mEntityToGO[Entity].GetName());
			mEntityToGO.erase(Entity);

			mEntityToGO[Entity].Destroy();

			//mRegistry.destroy(Entity);
		}

		mDeleteList.clear();
	}

	std::string GOFactory::CreateName(std::string name)
	{
		std::string goName = name;
		int count = 1;
		while (mNameToEntity.count(goName) != 0)
		{
			goName = name + "_" + std::to_string(count);
			count++;
		}

		return goName;
	}

	void GOFactory::VisitComponents(Entity entity, ComponentVisitor visitor)
	{
		// Go through every registered component
		for (auto&& [type_id, storage] : mRegistry.storage())
		{
			if (!storage.contains(entity))
			{
				continue; // entity does not have this component
			}

			// Each component for this GameObject is here
			std::cout << storage.type().name() << std::endl;
			std::string componentName(storage.type().name());

			rttr::type componentType = rttr::type::get_by_name(componentName);
			if (!componentType)
			{
				SLICE_LOG_ERROR("Component is not registered");
				continue;
			}

			auto it = mComponentGetters.find(type_id);
			if (it == mComponentGetters.end())
			{
				SLICE_LOG_ERROR("Component does not have a getter");
				// assert?

				continue;
			}

			rttr::variant componentData = it->second(mRegistry, entity);

			if (!componentData.is_valid())
			{
				SLICE_LOG_ERROR("Unable to retrieve component data");
				continue;
			}

			// call the std function
			visitor(componentType, componentData);
		}

	}

	//rttr::instance GetInstance(entt::id_type id, entt::registry& reg, entt::entity e)
	//{
	//	auto it = InstanceGetterFunc.find(id);
	//	if (it != InstanceGetterFunc.end())
	//		return it->second(reg, e);

	//	std::cerr << "[RTTR] Getter not found for entt id: " << id << "\n";
	//	return rttr::instance(); // invalid
	//}
}