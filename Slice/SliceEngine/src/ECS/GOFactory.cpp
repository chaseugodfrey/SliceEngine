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

	
	GameObject GOFactory::CreateBlank()
	{
		Entity entity = mRegistry.create();
		GameObject go(mRegistry, entity);

		// default name 
		// or i pass in a variable
		//go.SetName(CreateName(name));
		//go.AddComponent<SliceEntity>(CreateName("GameObject"));
		mNameToEntity.insert(std::make_pair(go.GetName(), go.GetEntity()));
		mEntityToGO.insert(std::make_pair(go.GetEntity(), go));

		return go;
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

		go.AddComponent<SliceEntity>();
		go.GetComponent<SliceEntity>().mName = CreateName(name);

		//go.SetName(CreateName(name));
		mNameToEntity.insert(std::make_pair(go.GetName(), go.GetEntity()));
		mEntityToGO.insert(std::make_pair(go.GetEntity(), go));

		// Can add default components here like transform
		//mRegistry.emplace_or_replace<Transform>(go);
		go.AddComponent<Transform>();
		// Every entity created will keep this flag for easy pulling

		return go;
	}

	// Not tested yet
	GameObject GOFactory::CreateUIGO(std::string name)
	{
		//Entity go = mRegistry.create();

		Entity entity = mRegistry.create();
		GameObject go(mRegistry, entity);
		//go.SetName(CreateName(name));
		mNameToEntity.insert(std::make_pair(go.GetName(), go.GetEntity()));
		mEntityToGO.insert(std::make_pair(go.GetEntity(), go));

		// Can add default components here like UITransform

		// Every entity created will keep this flag for easy pulling
		//mRegistry.emplace<SliceEntity>(go);
		go.AddComponent<SliceEntity>();

		return go;

	}

	// Not Tested yet
	GameObject GOFactory::CloneGO(GameObject const& go)
	{
		Entity entity = mRegistry.create();
		GameObject newGO(mRegistry, entity);

		//newGO.SetName(CreateName(newGO.GetName()));

		// loop through every component cloner to clone the component onto the new entity
		for (auto& cloner : mComponentCloners)
		{
			cloner.second(mRegistry, go.GetEntity(), newGO.GetEntity());
		}

		////mNameToEntity.insert(std::make_pair("Test", newGO));
		////mEntityToGO.insert(std::make_pair(newGO.GetEntity(), newGO));

		return newGO;
	}

	GameObject GOFactory::GetGOByName(std::string name)
	{
		auto it = mNameToEntity.find(name);
		if (it != mNameToEntity.end())
		{
			return mEntityToGO[it->second];
		}
		return GameObject();
	}
	
	GameObject GOFactory::GetGOByEntity(Entity entity)
	{
		auto it = mEntityToGO.find(entity);
		if (it != mEntityToGO.end())
		{
			return it->second;
		}
		return GameObject();
	}

	Entity GOFactory::GetRootEntity()
	{
		return mRootEntity;
	}

	void GOFactory::Destroy(GameObject& go)
	{
		mDeleteList.insert(go.GetEntity());
	}

	void GOFactory::InitRootEntity()
	{
		mRootEntity = mRegistry.create();
		mRegistry.emplace<SceneGraph>(mRootEntity);
	}

	void GOFactory::SetParent(Entity baseEntity, Entity parentEntity)
	{
		auto& baseEntitySceneGraph = mRegistry.get<SceneGraph>(baseEntity);

		// if the base entity has a parent, then we want to unattach it from its current chain
		if (baseEntitySceneGraph.neighbours[SceneGraph::UP] != entt::null)
		{
			auto& parentSceneGraph = mRegistry.get<SceneGraph>(baseEntitySceneGraph.neighbours[SceneGraph::UP]);

			// get the left and right sibling
			Entity leftSibling = entt::null;
			Entity rightSibling = entt::null;

			// if there is a siblin on the left
			if (baseEntitySceneGraph.neighbours[SceneGraph::LEFT] != entt::null)
			{
				// then we wanna remove this entity from it's right
				leftSibling = baseEntitySceneGraph.neighbours[SceneGraph::LEFT];
			}

			// if there is a siblin on the right
			if (baseEntitySceneGraph.neighbours[SceneGraph::RIGHT] != entt::null)
			{
				rightSibling = baseEntitySceneGraph.neighbours[SceneGraph::RIGHT];
			}

			// I ALMOST FORGOT
			// check if the current entity that is being re-parented is the direct DOWN child of it's current parent
			// if it is then it's right sibling would be the new down child since the left most child in the link list is the direct child of the parent
			if (parentSceneGraph.neighbours[SceneGraph::DOWN] == baseEntity)
			{
				if (rightSibling != entt::null)
				{
					// set the right sibling to the new down
					parentSceneGraph.neighbours[SceneGraph::DOWN] = rightSibling;
				}
				else
				{
					// if theres no right sibling mean the parent entity will no longer have a child
					parentSceneGraph.neighbours[SceneGraph::DOWN] = entt::null;
				}
				
			}

			// there is left
			if (leftSibling != entt::null)
			{
				auto& leftSiblingSceneGraph = mRegistry.get<SceneGraph>(leftSibling);
				// if there is a right then the right will be this entity's new right sibling
				if (rightSibling != entt::null)
				{
					leftSiblingSceneGraph.neighbours[SceneGraph::RIGHT] = rightSibling;
				}
				else
				{
					// else then right sibling becomes null 
					leftSiblingSceneGraph.neighbours[SceneGraph::RIGHT] = entt::null;
				}
			}

			if (rightSibling != entt::null)
			{
				auto& rightSiblingSceneGraph = mRegistry.get<SceneGraph>(rightSibling);
				
				// if there is a left sibling
				if (leftSibling != entt::null)
				{
					rightSiblingSceneGraph.neighbours[SceneGraph::LEFT] = leftSibling;
				}
				else
				{
					rightSiblingSceneGraph.neighbours[SceneGraph::LEFT] = entt::null;
				}
			}
		
			// idk if i need to but ill set the base entity's UP to null so we can treat it as a brand new entity beingg parented
			baseEntitySceneGraph.neighbours[SceneGraph::UP] = entt::null;
		}

		// if it has no parent / after we unattach it from it's current sibling list
		// we can treat it like a new entity if it had a parent previously

		Entity parent = entt::null;

		// Parenting to root entity
		if (parentEntity == entt::null)
		{
			// idk tbh incase they want to unparent and set it back to root node
			// then parentEntity would be a null
			parent = mRootEntity;
		}
		// Parenting to another entity
		else
		{
			if (!mRegistry.valid(parentEntity))
			{
				return;
			}

			parent = parentEntity;
		}

		auto& parentSceneGraph = mRegistry.get<SceneGraph>(parent);
		// if its the first entity being added to this scene graph as a child
		if (parentSceneGraph.neighbours[SceneGraph::DOWN] == entt::null)
		{
			// set the parent's down entity to the new entity
			parentSceneGraph.neighbours[SceneGraph::DOWN] = baseEntity;

			// set the new entity's up to the parent
			baseEntitySceneGraph.neighbours[SceneGraph::UP] = parent;
		}
		else
		{
			Entity childEntity = parentSceneGraph.neighbours[SceneGraph::DOWN];

			// if its not the first child in the parent scene graph then look through the children
			while (mRegistry.get<SceneGraph>(childEntity).neighbours[SceneGraph::RIGHT] != entt::null)
			{
				childEntity = mRegistry.get<SceneGraph>(childEntity).neighbours[SceneGraph::RIGHT];
			}

			// set the last child's right to the new entity
			auto& lastChildSceneGraph = mRegistry.get<SceneGraph>(childEntity);
			lastChildSceneGraph.neighbours[SceneGraph::RIGHT] = baseEntity;

			// Set the left of the new entity to the last child so its a double linked list
			// its right will remain as a null entt
			baseEntitySceneGraph.neighbours[SceneGraph::LEFT] = childEntity;

			//NOTE: I dont know if i should also set the UP to the parent entity. I'll do it for now
			// TODO: Check w chase
			baseEntitySceneGraph.neighbours[SceneGraph::UP] = parent;

		}

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

				// current issue is that 
				// SceneGraph storagei  ssaying that
				// entity 1 has a scene graph component
				// but it should not have it

				std::string componentName(storage.type().name());

				rttr::type componentType = rttr::type::get_by_name(componentName);
				if (!componentType)
				{
					SLICE_LOG_ERROR(std::string(storage.type().name()) + " is not registered");
					continue;
				}

				auto it = mComponentGetters.find(type_id);
				if (it == mComponentGetters.end())
				{
					SLICE_LOG_ERROR(std::string(storage.type().name()) + " does not have a getter");
					// assert?

					continue;
				}

				rttr::variant componentData = it->second(mRegistry, entity);

				if (!componentData.is_valid())
					continue;

				for (const auto& property : componentType.get_properties())
				{
					rttr::variant value = property.get_value(componentData);

					if (!value.is_valid())
						continue;

					// Print based on type
					if (value.is_type<int>())
						std::cout << property.get_name() << " = " << value.get_value<int>() << std::endl;
					else if (value.is_type<float>())
						std::cout << property.get_name() << " = " << value.get_value<float>() << std::endl;
					else if (value.is_type<double>())
						std::cout << property.get_name() << " = " << value.get_value<double>() << std::endl;
					else if (value.is_type<std::array<uint32_t, 4>>())
					{
						auto arr = value.get_value<std::array<uint32_t, 4>>();
						std::cout << property.get_name() << " = [";
						for (size_t i = 0; i < arr.size(); ++i)
							std::cout << arr[i] << (i + 1 < arr.size() ? ", " : "");
						std::cout << "]" << std::endl;
					}
					else if (value.is_type<glm::vec3>())
					{
						glm::vec3 v = value.get_value<glm::vec3>();
						std::cout << property.get_name() << " = ("
							<< v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
					}
					else
					{
						std::cout << property.get_name() << " = <unsupported type>" << std::endl;
					}
				}
			}
			//auto type = Registry::visi
		}
	}

	void GOFactory::UpdateName(std::string newName, Entity entity)
	{
		auto it = mEntityToGO.find(entity);
		if (it != mEntityToGO.end())
		{
			std::string oldName = it->second.GetName();
			//it->second.SetName(CreateName(newName)); changing name should be done in GO
			// update the name to entity map
			mNameToEntity.erase(oldName);
			mNameToEntity.insert(std::make_pair(newName, entity));
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

	void GOFactory::EmplaceComponents(Entity entity, const rttr::variant& componentVariant)
	{
		rttr::type componentType = componentVariant.get_type();
		std::string typeName = componentType.get_name().to_string();

		//if (componentVariant.is_type<std::shared_ptr<void>>())
		//{
		//	auto ptr_variant = componentVariant.convert<std::shared_ptr<void>>();
		//	if (ptr_variant)
		//	{
		//		componentType = rttr
		//	}
		//}

		auto it = mComponentEmplacer.find(componentType);
		if (it != mComponentEmplacer.end())
		{
			const ComponentEmplacer& emplaceFunction = it->second;
			emplaceFunction(mRegistry, entity, componentVariant);

			return;
		}

		auto it2 = mCESmartPtr.find(componentType);
		if (it2 != mCESmartPtr.end())
		{
			//const ComponentEmplacer& emplaceFunction = it->second;
			it2->second(mRegistry, entity, componentVariant);

			return;
		}

		
		
		SLICE_LOG_ERROR("COMPONENT HAS NO EMPLACER");
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