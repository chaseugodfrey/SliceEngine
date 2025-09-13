#include <pch.h>
#include "GOFactory.h"
#include "ECSTypes.h"


namespace SliceEngine
{
	GOFactory::GOFactory()
	{

	}

	GOFactory::~GOFactory()
	{

	}

	GameObject& GOFactory::CreateGO(std::string name)
	{
		GameObject go(*mRegistry);
		go.SetName(CreateName(name));
		mNameToEntity.insert(std::make_pair(go.GetName(), go.GetEntity()));
		mEntityToGO.insert(std::make_pair(go.GetEntity(), go));

		// Can add default components here like transform
		go.AddComponent<Transform>();

		// Every entity created will keep this flag for easy pulling
		go.AddComponent<SliceEntity>();

		return go;
	}

	GameObject& GOFactory::CreateUIGO(std::string name)
	{
		GameObject go(*mRegistry);
		go.SetName(CreateName(name));
		mNameToEntity.insert(std::make_pair(go.GetName(), go.GetEntity()));
		mEntityToGO.insert(std::make_pair(go.GetEntity(), go));

		// Can add default components here like UITransform

		// Every entity created will keep this flag for easy pulling
		go.AddComponent<SliceEntity>();

		return go;

	}

	GameObject GOFactory::CloneGO(GameObject& go)
	{
		GameObject newGO(*mRegistry);
		
		newGO.SetName(CreateName(go.GetName()));

		// loop through every component cloner to clone the component onto the new entity
		for (auto& cloner : mComponentCloners)
		{
			cloner.second(*mRegistry, go.GetEntity(), newGO.GetEntity());
		}

		mNameToEntity.insert(std::make_pair(newGO.GetName(), newGO.GetEntity()));
		mEntityToGO.insert(std::make_pair(newGO.GetEntity(), newGO));

		return newGO;
	}

	void GOFactory::Destroy(Entity const& entity)
	{
		mDeleteList.insert(entity);
	}

	void GOFactory::Destroy(GameObject& go)
	{
		mDeleteList.insert(go.GetEntity());
	}

	void GOFactory::TestLoop()
	{
		auto entityView = mRegistry->view<SliceEntity>();
		for (auto entity : entityView)
		{
			std::cout << mEntityToGO[entity].GetName() << std::endl;
			
			
			for (auto&& [type_id, storage] : mRegistry->storage())
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
			mEntityToGO[Entity].Destroy();

			// erase from the maps
			mNameToEntity.erase(mEntityToGO[Entity].mName);
			mEntityToGO.erase(Entity);

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
}