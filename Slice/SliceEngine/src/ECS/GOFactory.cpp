/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			GOFactory.cpp
 author:		Gideon Francis
 email:			g.francis@digipen.edu
 brief:			Handles things related to GameObjects

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "GOFactory.h"
#include "ECS/ECSTypes.h"
#include "../Core/ComponentEventHandler.h"


namespace SliceEngine
{
	GOFactory::GOFactory()
	{
		mRegistry.on_construct<ColliderShape>().connect<&OnColliderShapeAdded>();
		mRegistry.on_destroy<ColliderShape>().connect<&OnColliderShapeRemoved>();
		mRegistry.on_construct<RigidBody>().connect<&OnRigidBodyAdded>();
		mRegistry.on_destroy<RigidBody>().connect<&OnRigidBodyRemoved>();
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
		go.AddComponent<SceneGraph>();
		SetParent(go.GetEntity());
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

		// Networking stuff




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
		Destroy(go.GetEntity());
	}

	void GOFactory::Destroy(entt::entity entity)
	{
		auto go = GetGOByEntity(entity);

		//Check children and destroy them too
		if(go.HasComponent<SceneGraph>())
		{
			auto& sceneGraph = go.GetComponent<SceneGraph>();
			Entity child = sceneGraph.neighbours[SceneGraph::DOWN];
			while (child != entt::null)
			{
				auto& childSceneGraph = mRegistry.get<SceneGraph>(child);
				Entity nextSibling = childSceneGraph.neighbours[SceneGraph::RIGHT];
				Destroy(child);
				child = nextSibling;
			}
		}
		else
		{
			SLICE_LOG_ERROR("Trying to destroy entity that does not have a scene graph component");
		}
		mDeleteList.insert(entity);
	}

	void GOFactory::InitRootEntity()
	{
		mRootEntity = mRegistry.create();
		//auto& tr = mRegistry.emplace<Transform>(mRootEntity);
		mRegistry.emplace<SceneGraph>(mRootEntity);
	}

	void GOFactory::Unparent(Entity entity)
	{
		// idk if i need to but ill set the base entity's UP to null so we can treat it as a brand new entity beingg parented
		auto& scene_graph = mRegistry.get<SceneGraph>(entity);
		auto prev_parent_entity = scene_graph.neighbours[SceneGraph::UP];

		if (prev_parent_entity == entt::null)
			return;

		auto& parent_scene_graph = mRegistry.get<SceneGraph>(prev_parent_entity);
		auto grandparent_entity = parent_scene_graph.neighbours[SceneGraph::UP];

		SetParent(entity, grandparent_entity);
	}

	void GOFactory::SetParent(Entity entity, Entity parentEntity)
	{
		auto& scene_graph = mRegistry.get<SceneGraph>(entity);
		auto prev_parent_entity = scene_graph.neighbours[SceneGraph::UP];

		// if the base entity has a parent, then we want to unattach it from its current chain
		if (prev_parent_entity != entt::null)
		{
			auto& prev_parent_scene_graph = mRegistry.get<SceneGraph>(prev_parent_entity);

			// get the left and right sibling
			Entity left_entity = entt::null;
			Entity right_entity = entt::null;

			// if there is a siblin on the left
			if (scene_graph.neighbours[SceneGraph::LEFT] != entt::null)
			{
				// then we wanna remove this entity from it's right
				left_entity = scene_graph.neighbours[SceneGraph::LEFT];
			}

			// if there is a siblin on the right
			if (scene_graph.neighbours[SceneGraph::RIGHT] != entt::null)
			{
				right_entity = scene_graph.neighbours[SceneGraph::RIGHT];
			}

			// I ALMOST FORGOT
			// check if the current entity that is being re-parented is the direct DOWN child of it's current parent
			// if it is then it's right sibling would be the new down child since the left most child in the link list is the direct child of the parent
			if (prev_parent_scene_graph.neighbours[SceneGraph::DOWN] == entity)
			{
				if (right_entity != entt::null)
				{
					// set the right sibling to the new down
					prev_parent_scene_graph.neighbours[SceneGraph::DOWN] = right_entity;
				}
				else
				{
					// if theres no right sibling mean the parent entity will no longer have a child
					prev_parent_scene_graph.neighbours[SceneGraph::DOWN] = entt::null;
				}

			}

			// there is left
			if (left_entity != entt::null)
			{
				//Set itself's left to null
				scene_graph.neighbours[SceneGraph::LEFT] = entt::null;

				auto& left_scene_graph = mRegistry.get<SceneGraph>(left_entity);
				// if there is a right then the right will be this entity's new right sibling
				if (right_entity != entt::null)
				{
					left_scene_graph.neighbours[SceneGraph::RIGHT] = right_entity;
				}
				else
				{
					// else then right sibling becomes null 
					left_scene_graph.neighbours[SceneGraph::RIGHT] = entt::null;
				}
			}

			if (right_entity != entt::null)
			{
				//Set itself's right to null
				scene_graph.neighbours[SceneGraph::RIGHT] = entt::null;

				auto& right_scene_graph = mRegistry.get<SceneGraph>(right_entity);

				// if there is a left sibling
				if (left_entity != entt::null)
				{
					right_scene_graph.neighbours[SceneGraph::LEFT] = left_entity;
				}
				else
				{
					right_scene_graph.neighbours[SceneGraph::LEFT] = entt::null;
				}
			}

			--prev_parent_scene_graph.child_count;
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

		auto& parent_scene_graph = mRegistry.get<SceneGraph>(parent);
		// if its the first entity being added to this scene graph as a child
		if (parent_scene_graph.neighbours[SceneGraph::DOWN] == entt::null)
		{
			// set the parent's down entity to the new entity
			parent_scene_graph.neighbours[SceneGraph::DOWN] = entity;

			// set the new entity's up to the parent
			scene_graph.neighbours[SceneGraph::UP] = parent;
		}
		else
		{
			Entity child_entity = parent_scene_graph.neighbours[SceneGraph::DOWN];

			// if its not the first child in the parent scene graph then look through the children
			while (mRegistry.get<SceneGraph>(child_entity).neighbours[SceneGraph::RIGHT] != entt::null)
			{
				child_entity = mRegistry.get<SceneGraph>(child_entity).neighbours[SceneGraph::RIGHT];
			}

			// set the last child's right to the new entity
			auto& last_child_scene_graph = mRegistry.get<SceneGraph>(child_entity);
			last_child_scene_graph.neighbours[SceneGraph::RIGHT] = entity;

			// Set the left of the new entity to the last child so its a double linked list
			// its right will remain as a null entt
			scene_graph.neighbours[SceneGraph::LEFT] = child_entity;
		}

		++parent_scene_graph.child_count;
		scene_graph.neighbours[SceneGraph::UP] = parent;

	}

	void GOFactory::SetSiblingIndex(Entity targetEntity, int pos)
	{
		auto& target_scene_graph = mRegistry.get<SceneGraph>(targetEntity);

		auto parent_entity = target_scene_graph.neighbours[SceneGraph::UP];
		auto& parent_scene_graph = mRegistry.get<SceneGraph>(parent_entity);

		auto child_entity = parent_scene_graph.neighbours[SceneGraph::DOWN];

		if (child_entity == entt::null)
			return;

		// get which is lower so that pos doesnt go out of bounds
		pos = std::min((int)parent_scene_graph.child_count, pos);

		for (int i = 0; i < pos; i++)
		{
			auto& child_scene_graph = mRegistry.get<SceneGraph>(child_entity);
			child_entity = child_scene_graph.neighbours[SceneGraph::RIGHT];
		}

		// attach loose ends first
		// get left and right of target entity
		auto sibling_before_left = target_scene_graph.neighbours[SceneGraph::LEFT];
		auto sibling_before_right = target_scene_graph.neighbours[SceneGraph::RIGHT];

		if (sibling_before_left != entt::null)
		{
			auto& bl_scene_graph = mRegistry.get<SceneGraph>(sibling_before_left);
			bl_scene_graph.neighbours[SceneGraph::RIGHT] = sibling_before_right;

			if (sibling_before_right != entt::null)
			{
				auto& br_scene_graph = mRegistry.get<SceneGraph>(sibling_before_right);
				br_scene_graph.neighbours[SceneGraph::LEFT] = sibling_before_left;
			}
		}

		// attach new siblings
		auto& destination_scene_graph = mRegistry.get<SceneGraph>(child_entity);
		auto sibling_after_left = destination_scene_graph.neighbours[SceneGraph::LEFT];

		// if null, then will be first 
		if (sibling_after_left != entt::null)
		{
			auto& al_scene_graph = mRegistry.get<SceneGraph>(sibling_after_left);
			al_scene_graph.neighbours[SceneGraph::RIGHT] = targetEntity;
			target_scene_graph.neighbours[SceneGraph::LEFT] = sibling_after_left;
			target_scene_graph.neighbours[SceneGraph::RIGHT] = child_entity;
		}

		else
		{
			parent_scene_graph.neighbours[SceneGraph::DOWN] = targetEntity;
			target_scene_graph.neighbours[SceneGraph::RIGHT] = child_entity;
		}

		destination_scene_graph.neighbours[SceneGraph::LEFT] = targetEntity;

	}

	void GOFactory::BuildSceneGraph()
	{
		auto view = mRegistry.view<SceneGraph>();
		auto scene_root_entity = entt::entity{ 0 };

		for (auto entity : view)
		{
			if (entity == scene_root_entity)
				continue;

			auto& graph = mRegistry.get<SceneGraph>(entity);
			SetParent(entity, graph.neighbours[SceneGraph::UP]);
		}
	}

	GameObject GOFactory::CreateGO_Box()
	{
		auto go = CreateGO("GameObject");
		go.AddComponent<Renderer>();
		go.AddComponent<ColliderShape>();
		go.AddComponent<RigidBody>();

		return go;
	}

	GameObject GOFactory::CreateGO_Cam()
	{
		auto go = CreateGO("Camera");
		go.AddComponent<Camera>();
		return go;
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
				{
					SLICE_LOG_ERROR(std::string(componentType.get_name().to_string() + " componentData is invalid "));
					continue;
				}
					

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
					else if (value.get_type() == rttr::type::get<EntityID>() ||
						value.get_type().is_derived_from(rttr::type::get<EntityID>()))
					{
						EntityID eid = value.get_value<EntityID>();
						std::cout << property.get_name() << " = " << eid.value << std::endl;
					}
					else if (value.is_type<std::array<Entity, 4>>())
					{
						auto arr = value.get_value<std::array<Entity, 4>>();
						std::cout << property.get_name() << " = [";
						for (size_t i = 0; i < arr.size(); ++i)
							std::cout << static_cast<uint32_t>(arr[i]) << (i + 1 < arr.size() ? ", " : "");
						std::cout << "]" << std::endl;
					}
					else if (value.is_type<glm::vec3>())
					{
						glm::vec3 v = value.get_value<glm::vec3>();
						std::cout << property.get_name() << " = ("
							<< v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
					}
					else if (value.get_type() == rttr::type::get<uint32_t>() ||
						value.get_type().is_derived_from(rttr::type::get<uint32_t>()))
					{
						uint32_t u = value.get_value<uint32_t>();
						std::cout << property.get_name() << " = " << u << std::endl;
					}
					else if (value.is_type<std::string>() ||
						value.get_type().is_derived_from(rttr::type::get<std::string>()))
					{
						std::string str = value.get_value<std::string>();
						std::cout << property.get_name() << " = \"" << str << "\"" << std::endl;
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
		for (auto Entity : mDeleteList)
		{
			if (mEntityToGO[Entity].HasComponent<SceneGraph>())
			{
				SceneGraphDelete(Entity);
			}

			// idk if its okay to destroy EnTT entity before clearing from map
			// but ill leave it like this for now
			mEntityToGO[Entity].Destroy();

			// erase from the maps
			mNameToEntity.erase(mEntityToGO[Entity].GetName());
			mEntityToGO.erase(Entity);


			//mRegistry.destroy(Entity);
		}

		mDeleteList.clear();
	}

	void GOFactory::SceneGraphDelete(Entity entity)
	{
		auto& sceneGraph = mEntityToGO[entity].GetComponent<SceneGraph>();
		//Check for siblings
		if (sceneGraph.neighbours[SceneGraph::LEFT] != entt::null && sceneGraph.neighbours[SceneGraph::RIGHT] != entt::null)
		{
			if(mEntityToGO[sceneGraph.neighbours[SceneGraph::LEFT]].HasComponent<SceneGraph>() && mEntityToGO[sceneGraph.neighbours[SceneGraph::RIGHT]].HasComponent<SceneGraph>())
			{
				auto& leftSiblingGraph = mEntityToGO[sceneGraph.neighbours[SceneGraph::LEFT]].GetComponent<SceneGraph>();
				auto& rightSiblingGraph = mEntityToGO[sceneGraph.neighbours[SceneGraph::RIGHT]].GetComponent<SceneGraph>();

				leftSiblingGraph.neighbours[SceneGraph::RIGHT] = sceneGraph.neighbours[SceneGraph::RIGHT];
				rightSiblingGraph.neighbours[SceneGraph::LEFT] = sceneGraph.neighbours[SceneGraph::LEFT];
			}
		}
		else if(sceneGraph.neighbours[SceneGraph::LEFT] != entt::null)
		{
			if (mEntityToGO[sceneGraph.neighbours[SceneGraph::LEFT]].HasComponent<SceneGraph>())
			{
				auto& leftSiblingGraph = mEntityToGO[sceneGraph.neighbours[SceneGraph::LEFT]].GetComponent<SceneGraph>();
				leftSiblingGraph.neighbours[SceneGraph::RIGHT] = entt::null;
			}
		}
		else if (sceneGraph.neighbours[SceneGraph::RIGHT] != entt::null)
		{
			if (mEntityToGO[sceneGraph.neighbours[SceneGraph::RIGHT]].HasComponent<SceneGraph>())
			{
				auto& rightSiblingGraph = mEntityToGO[sceneGraph.neighbours[SceneGraph::RIGHT]].GetComponent<SceneGraph>();
				rightSiblingGraph.neighbours[SceneGraph::LEFT] = entt::null;
			}
		}

		//Re-set parent down if needed
		if (sceneGraph.neighbours[SceneGraph::UP] != entt::null)
		{
			if (sceneGraph.neighbours[SceneGraph::UP] == mRootEntity)
			{
				auto& parentGraph = mRegistry.get<SceneGraph>(mRootEntity);
				if(parentGraph.neighbours[SceneGraph::DOWN] == entity)
				{
					parentGraph.neighbours[SceneGraph::DOWN] = sceneGraph.neighbours[SceneGraph::RIGHT];
				}
				//parentGraph.neighbours[SceneGraph::DOWN] = sceneGraph.neighbours[SceneGraph::RIGHT];
			}

			else if (mEntityToGO[sceneGraph.neighbours[SceneGraph::UP]].HasComponent<SceneGraph>())
			{
				auto& parentGraph = mEntityToGO[sceneGraph.neighbours[SceneGraph::UP]].GetComponent<SceneGraph>();
				parentGraph.neighbours[SceneGraph::DOWN] = sceneGraph.neighbours[SceneGraph::RIGHT];
			}
		}
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