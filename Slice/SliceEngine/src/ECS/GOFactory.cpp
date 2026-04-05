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
#include "../Core/ComponentModified.h"
#include "../Graphics/TransformHelper.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

//Somehow kinda funny i need to include 2 files inorder to get the resource manager instance
//#include "../Resource/ResourceManager.h"
#include "../Core/Core.h"

namespace SliceEngine
{
	GOFactory::GOFactory()
	{
		mRegistry.on_construct<ColliderShape>().connect<&OnColliderShapeAdded>();
		//mRegistry.on_destroy<ColliderShape>().connect<&OnColliderShapeRemoved>();
		mRegistry.on_construct<RigidBody>().connect<&OnRigidBodyAdded>();
		mRegistry.on_destroy<RigidBody>().connect<&OnRigidBodyRemoved>();
		//mRegistry.on_update<SliceEntity>().connect<&NotifySliceEntityModified>();
	}

	GOFactory::~GOFactory()
	{
		SLICE_LOG_DEBUG("test");
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

	//	//std::cout << "Creating blank GO for prefab " << (uint32_t)entity << std::endl;

		return go;
	}

	GameObject GOFactory::CreateBlanker()
	{
		Entity entity = mRegistry.create();
		GameObject go(mRegistry, entity);
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
		go.GetComponent<SceneGraph>().entity_id = (uint32_t)entity;
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
		return CloneGO(go, entt::null);
	}

	GameObject GOFactory::CloneGO(GameObject const& go, Entity parentEntity)
	{
		Entity entity = mRegistry.create();
		GameObject newGO(mRegistry, entity);

		mNameToEntity.insert(std::make_pair(newGO.GetName(), newGO.GetEntity()));
		mEntityToGO.insert(std::make_pair(newGO.GetEntity(), newGO));

		// loop through every component cloner to clone the component onto the new entity
		for (auto& cloner : mComponentCloners)
		{
			cloner.second(mRegistry, go.GetEntity(), newGO.GetEntity());
		}

		newGO.RemoveComponent<SceneGraph>();
		newGO.SetName(CreateName(go.GetName()));
		newGO.AddComponent<SceneGraph>();
		newGO.GetComponent<SceneGraph>().entity_id = (uint32_t)entity;

		// need preserve transform if not it passes away
		if (newGO.HasComponent<Transform>())
		{
			auto& tr = newGO.GetComponent<Transform>();
			
			// Get the original parent and new parent
			Entity oldParent = entt::null;
			if (go.HasComponent<SceneGraph>())
				oldParent = go.GetComponent<SceneGraph>().neighbours[SceneGraph::UP];

			Entity newParent = parentEntity == entt::null ? mRootEntity : parentEntity;

			// Calculate the original world transform matrix
			glm::mat4 localMtx = glm::translate(glm::mat4(1.0f), tr.position) *
				glm::mat4_cast(tr.rotation) *
				glm::scale(glm::mat4(1.0f), tr.scale);

			glm::mat4 worldMtx;
			if (oldParent != entt::null && mRegistry.any_of<Transform>(oldParent))
			{
				auto& tr_old_par = mRegistry.get<Transform>(oldParent);
				worldMtx = tr_old_par.transform * localMtx;
			}
			else
			{
				worldMtx = localMtx;
			}

			// Set the parent first (this might mess up local transform)
			SetParent(newGO.GetEntity(), parentEntity);

			// Now recalculate the local transform relative to the new parent to maintain world position
			if (mRegistry.any_of<Transform>(newParent))
			{
				auto& tr_new_par = mRegistry.get<Transform>(newParent);
				glm::mat4 relMtx = glm::inverse(tr_new_par.transform) * worldMtx;

				glm::vec3 translation, scale, skew;
				glm::vec4 perspective;
				glm::quat rotation;
				glm::decompose(relMtx, scale, rotation, translation, skew, perspective);

				tr.position = translation;
				tr.rotation = rotation;
				tr.scale = scale;

				// Update the matrices so they aren't stale for children
				tr.transform_local = relMtx;
				tr.transform = worldMtx;
			}
		}
		else
		{
			SetParent(newGO.GetEntity(), parentEntity);
		}

		// clone children
		if (go.HasComponent<SceneGraph>())
		{
			auto& sceneGraph = go.GetComponent<SceneGraph>();
			Entity child = sceneGraph.neighbours[SceneGraph::DOWN];
			while (child != entt::null)
			{
				GameObject childGO = GetGOByEntity(child);
				CloneGO(childGO, newGO.GetEntity());

				auto& childSceneGraph = mRegistry.get<SceneGraph>(child);
				child = childSceneGraph.neighbours[SceneGraph::RIGHT];
			}
		}

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

	Entity GOFactory::GetEntityWithTag(std::string const& tag)
	{
		auto view = mRegistry.view<SceneGraph>();

		for (auto entity : view)
		{
			GameObject go = mEntityToGO[entity];
			if (go.HasComponent<SliceEntity>() &&
				go.GetComponent<SliceEntity>().mTag == tag)
			{
				return entity;
			}
		}

		return entt::null;
	}

	std::vector<Entity> GOFactory::GetEntitiesWithTag(std::string const& tag)
	{
		std::vector<Entity> result;

		// Arbitrary number as idk what to expect
		result.reserve(64);

		auto view = mRegistry.view<SceneGraph>();

		for (auto entity : view)
		{
			if (entity == Entity(0))
			{
				continue;
			}
			GameObject go = mEntityToGO[entity];
			if (go.HasComponent<SliceEntity>() &&
				go.GetComponent<SliceEntity>().mTag == tag)
			{
				result.push_back(entity);
			}
		}


		return result;
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
		////std::cout << "Destryoing in go factory: " << (uint32_t)entity << std::endl;

		if (mDeleteList.contains(entity))
		{
			//SLICE_LOG_WARNING("Trying to destroy entity that is already marked for deletion");
			//return;
		}

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
			//SLICE_LOG_ERROR("Trying to destroy entity that does not have a scene graph component");
		}
		mDeleteList.insert(entity);
	}

	void GOFactory::InitRootEntity()
	{
		mRootEntity = mRegistry.create();
		mRegistry.emplace<Transform>(mRootEntity);
		mRegistry.emplace<SceneGraph>(mRootEntity);
		SLICE_LOG("Init Root");
	}

	void GOFactory::RemoveFromNameMap(Entity entity)
	{
		GameObject go = GetGOByEntity(entity);

		mNameToEntity.erase(go.GetName());
		//mEntityToGO.insert(std::make_pair(go.GetEntity(), go));
	}

	void GOFactory::AddToNameMap(Entity entity)
	{
		GameObject go = GetGOByEntity(entity);
		if (mNameToEntity.find(go.GetName()) == mNameToEntity.end())
		{
			mNameToEntity[go.GetName()] = entity;
		}
		else
		{
			// already exist in the map
			go.GetComponent<SliceEntity>().mName = CreateName(go.GetName());
			mNameToEntity[go.GetName()] = entity;
		}

	}

	void GOFactory::RemoveFromNameMap(std::string name)
	{
		if (mNameToEntity.find(name) == mNameToEntity.end())
		{
			return;
		}

		mNameToEntity.erase(name);
	}

	bool GOFactory::isDescendant(Entity target, Entity dest)
	{
		if(dest == entt::null)
		{
			return false;
		}
		auto& destSceneGraph = mRegistry.get<SceneGraph>(dest);
		//auto& destSceneGraph = mRegistry.get<SceneGraph>(dest);
		//Check direct children
		auto parent = destSceneGraph.neighbours[SceneGraph::UP];
		//Checking the right siblings of the child until null
		while(parent != entt::null)
		{
			if(parent == target)
			{
				return true;
			}

			//Recursively check the child too (This is wrong)
			if(isDescendant(target, parent))
			{
				return true;
			}
			parent = mRegistry.get<SceneGraph>(parent).neighbours[SceneGraph::UP];
		}

		return false;
	}

	bool GOFactory::Unparent(Entity entity)
	{
		// idk if i need to but ill set the base entity's UP to null so we can treat it as a brand new entity beingg parented
		auto& scene_graph = mRegistry.get<SceneGraph>(entity);
		auto prev_parent_entity = scene_graph.neighbours[SceneGraph::UP];

		if (prev_parent_entity == entt::null)
			return false;

		auto& parent_scene_graph = mRegistry.get<SceneGraph>(prev_parent_entity);
		auto grandparent_entity = parent_scene_graph.neighbours[SceneGraph::UP];

		SetParent(entity, grandparent_entity);
		return true;
	}

	bool GOFactory::SetParent(Entity entity, Entity parentEntity)
	{
		if(isDescendant(entity, parentEntity))
		{
			SLICE_LOG_ERROR("Trying to set parent to a descendant entity, do not do it");
			return false;
		}


		auto& scene_graph = mRegistry.get<SceneGraph>(entity);
		auto prev_parent_entity = scene_graph.neighbours[SceneGraph::UP];
		//Check if there's even a need to update the parent.
		if (prev_parent_entity == parentEntity && prev_parent_entity != entt::null)
		{
			SLICE_LOG_WARNING("Parenting to self. Does nothing.");
			return false;
		}
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
		}

		// if it has no parent / after we unattach it from it's current sibling list
		// we can treat it like a new entity if it had a parent previously

		Entity new_parent = entt::null;

		// Parenting to root entity
		if (parentEntity == entt::null)
		{
			// idk tbh incase they want to unparent and set it back to root node
			// then parentEntity would be a null
			new_parent = mRootEntity;
		}
		// Parenting to another entity
		else
		{
			if (!mRegistry.valid(parentEntity))
			{
				return false;
			}

			new_parent = parentEntity;
		}

		auto& parent_scene_graph = mRegistry.get<SceneGraph>(new_parent);
		// if its the first entity being added to this scene graph as a child
		if (parent_scene_graph.neighbours[SceneGraph::DOWN] == entt::null)
		{
			// set the parent's down entity to the new entity
			parent_scene_graph.neighbours[SceneGraph::DOWN] = entity;

			// set the new entity's up to the parent
			scene_graph.neighbours[SceneGraph::UP] = new_parent;
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

		//++parent_scene_graph.child_count;
		scene_graph.neighbours[SceneGraph::UP] = new_parent;

		UpdateTransformFromParent(entity, new_parent, prev_parent_entity);
		return true;
	}

	void GOFactory::SetNewSceneGraphLocation(Entity targetEntity, Entity rightEntity, Entity leftEntity, Entity parentEntity)
	{

		if (isDescendant(targetEntity, rightEntity))
		{
			SLICE_LOG_ERROR("Trying to set parent to a descendant entity, do not do it");
			return;
		}

		if (targetEntity == leftEntity)
		{
			SLICE_LOG_WARNING("Not changing SceneGraph at all");
			return;
		}
		
		//I think i can call SetParent here. Wait wtf i dont have the parent. NVM i have it now.
		SetParent(targetEntity, parentEntity);

		//Remove it from its current position no longer needed.
		SceneGraphDelete(targetEntity);
		auto& movedSceneGraph = mRegistry.get<SceneGraph>(targetEntity);

		//It is the left most entity (Need to update parent's down and right's left)
		if (leftEntity == entt::null)
		{
			auto& destRightGraph = mRegistry.get<SceneGraph>(rightEntity);
			//Check if parent is rootNode
			if (destRightGraph.neighbours[SceneGraph::UP] == GetRootEntity())
			{
				//Update rootNode's down to target entity
				mRegistry.get<SceneGraph>(GetRootEntity()).neighbours[SceneGraph::DOWN] = targetEntity;
			}
			else
			{
				auto& destRightParentGraph = mRegistry.get<SceneGraph>(destRightGraph.neighbours[SceneGraph::UP]);
				//Update parent's down to target entity
				destRightParentGraph.neighbours[SceneGraph::DOWN] = targetEntity;
			}
			//Update right entity's left to target entity
			destRightGraph.neighbours[SceneGraph::LEFT] = targetEntity;

			//Update the target entity itself (Remember to update its up too)
			movedSceneGraph.neighbours[SceneGraph::RIGHT] = rightEntity;
			movedSceneGraph.neighbours[SceneGraph::LEFT] = entt::null;
			movedSceneGraph.neighbours[SceneGraph::UP] = destRightGraph.neighbours[SceneGraph::UP];
		}

		//It is the right most entity (Need to update left's right)
		else if (rightEntity == entt::null)
		{
			auto& destLeftGraph = mRegistry.get<SceneGraph>(leftEntity);
			//Update left entity's right to target entity
			destLeftGraph.neighbours[SceneGraph::RIGHT] = targetEntity;

			//Update the target entity itself (Remember to update its up too)
			movedSceneGraph.neighbours[SceneGraph::LEFT] = leftEntity;
			movedSceneGraph.neighbours[SceneGraph::UP] = destLeftGraph.neighbours[SceneGraph::UP];
		}
		//It is between two entities
		else
		{
			auto& destRightGraph = mRegistry.get<SceneGraph>(rightEntity);
			auto& destLeftGraph = mRegistry.get<SceneGraph>(leftEntity);

			//Update right entity's left to target entity
			destRightGraph.neighbours[SceneGraph::LEFT] = targetEntity;
			//Update left entity's right to target entity
			destLeftGraph.neighbours[SceneGraph::RIGHT] = targetEntity;

			//Update the target entity itself (Remember to update its up too)
			movedSceneGraph.neighbours[SceneGraph::RIGHT] = rightEntity;
			movedSceneGraph.neighbours[SceneGraph::LEFT] = leftEntity;
			movedSceneGraph.neighbours[SceneGraph::UP] = destLeftGraph.neighbours[SceneGraph::UP];
		}

		//Any more edge cases?
	}

	void GOFactory::UpdateTransformFromParent(Entity entity, Entity parent, Entity oldParent)
	{
		if (!mRegistry.any_of<Transform>(entity) || !mRegistry.any_of<Transform>(parent))
			return;

		auto& tr = mRegistry.get<Transform>(entity);
		auto& tr_par = mRegistry.get<Transform>(parent);

		// Build the world transform matrix from the current position/rotation/scale
		glm::mat4 localTransform = glm::translate(glm::mat4(1.0f), tr.position) *
			glm::mat4_cast(tr.rotation) *
			glm::scale(glm::mat4(1.0f), tr.scale);

		glm::mat4 worldTransform;
		if (oldParent != entt::null && mRegistry.any_of<Transform>(oldParent))
		{
			auto& tr_old_par = mRegistry.get<Transform>(oldParent);
			worldTransform = tr_old_par.transform * localTransform;
		}
		else
		{
			worldTransform = localTransform;
		}

		// Convert world transform to local space relative to parent
		auto mat = glm::inverse(tr_par.transform) * worldTransform;

		glm::vec3 translation, scale, skew;
		glm::vec4 perspective;
		glm::quat rotation;
		glm::decompose(mat, scale, rotation, translation, skew, perspective);

		tr.position = translation;
		tr.rotation = rotation;
		tr.scale = scale;
	}

	bool GOFactory::CheckValidName(Entity entity)
	{
		auto& name = mRegistry.get<SliceEntity>(entity).mName;

		if (mNameToEntity.find(name) != mNameToEntity.end())
		{
			if (mNameToEntity[name] == entity)
			{
				return true;
			}
		}
		return true;
	}

	void GOFactory::FactoryShutdown()
	{
		mRegistry.clear();
	}


	

	void GOFactory::BuildSceneGraph(std::unordered_map<uint32_t, uint32_t> map)
	{
		auto view = mRegistry.view<SceneGraph>();
		auto scene_root_entity = entt::entity{ 0 };

		//auto& parentSceneGraph = mRegistry.get<SceneGraph>(scene_root_entity);

		// update root entity's child
		//auto parentChildIt = map.find((uint32_t)parentSceneGraph.neighbours[SceneGraph::DOWN]);
		//if (parentChildIt != map.end())
		//{

		//	parentSceneGraph.neighbours[SceneGraph::DOWN] = (Entity)parentChildIt->second;
		//}

		for (auto entity : view)
		{
			auto& scene_graph = mRegistry.get<SceneGraph>(entity);

			if (entity == scene_root_entity)
				continue;

			if (scene_graph.neighbours[SceneGraph::UP] == scene_root_entity &&
				scene_graph.neighbours[SceneGraph::LEFT] == entt::null)
			{
				mRegistry.get<SceneGraph>(scene_root_entity).neighbours[SceneGraph::DOWN] = entity;
			}

			// update its own entity id
			uint32_t entityID = scene_graph.entity_id;
			auto entityIt = map.find(entityID);
			if (entityIt != map.end())
			{
				scene_graph.entity_id = entityIt->second;
			}

			// update the neighbouts
			for (size_t i = 0; i < scene_graph.DIRECTIONS; i++)
			{
				entt::entity key_entity = scene_graph.neighbours[i];
				if (key_entity == entt::null)
					continue;

				uint32_t key = entt::to_integral(key_entity);
				auto it = map.find(key);

				if (it != map.end())
				{
					entt::entity ent = static_cast<entt::entity>(it->second);

					scene_graph.neighbours[i] = ent;

				}
			}
		}
	}

	void GOFactory::ClearGameObjects()
	{
		auto view = mRegistry.view<SliceEntity>();
		mDeleteList.clear(); // might need it now again
		for (auto entity : view)
		{
			SLICE_LOG("Entity : " + std::to_string((uint32_t)entity));
			Destroy(entity);
		}

		mRegistry.get<SceneGraph>(mRootEntity).neighbours[SceneGraph::DOWN] = entt::null;
		//
		//mNameToEntity.clear();
		//mEntityToGO.clear();
		//mRegistry.clear();
	}

	GameObject GOFactory::CreateGO_Box()
	{
		auto go = CreateGO("GameObject");
		go.AddComponent<Renderer>();
		go.AddComponent<ColliderShape>(ColliderShape::BoxData{});
		go.AddComponent<RigidBody>();

		return go;
	}
	
	GameObject GOFactory::CreateGO_Sphere()
	{
		auto go = CreateGO("GameObject");
		go.AddComponent<Renderer>();
		go.GetComponent<Renderer>().modelHandle = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::SPHERE_DEFAULT);
		go.AddComponent<ColliderShape>(ColliderShape::SphereData{});
		go.AddComponent<RigidBody>();

		return go;
		//testing only
		//return CreateGO_Model((GUID)17518266545644652909);


	}

	GameObject GOFactory::CreateGO_Capsule()
	{
		auto go = CreateGO("GameObject");
		go.AddComponent<Renderer>();
		go.GetComponent<Renderer>().modelHandle = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::CAPSULE_DEFAULT);
		go.AddComponent<ColliderShape>(ColliderShape::CapsuleData{});
		go.AddComponent<RigidBody>();

		return go;
	}

	GameObject GOFactory::CreateGO_Cylinder()
	{
		auto go = CreateGO("GameObject");
		go.AddComponent<Renderer>();
		go.GetComponent<Renderer>().modelHandle = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)DefaultResourceIDs::CYLINDER_DEFAULT);
		go.AddComponent<ColliderShape>(ColliderShape::CylinderData{});
		go.AddComponent<RigidBody>();

		return go;
	}

	GameObject GOFactory::CreateGO_Cam()
	{
		auto go = CreateGO("Camera");
		go.AddComponent<Camera>();
		return go;
	}

	GameObject GOFactory::CreateGO_Light()
	{
		auto go = CreateGO("Light");
		go.AddComponent<Light>();
		go.GetComponent<Light>().type = Light::Light_Spot;
		return go;
	}

	GameObject GOFactory::CreateGO_Canvas()
	{
		auto canvas = CreateGO("Canvas");
		canvas.AddComponent<Canvas>();
		canvas.AddComponent<RectTransform>();
		auto& c_rect = canvas.GetComponent<RectTransform>();
		c_rect.width = 1920; c_rect.height = 1080; c_rect.pos_x = 0; c_rect.pos_y = 0;

		return canvas;
	}
	GameObject GOFactory::CreateGO_Image()
	{
		auto ui_ele = CreateGO("Image");
		ui_ele.AddComponent<RectTransform>();
		auto& ui_rect = ui_ele.GetComponent<RectTransform>();
		ui_rect.width = 100; ui_rect.height = 100; ui_rect.pos_x = 0; ui_rect.pos_y = 0;
		ui_ele.AddComponent<SpriteRenderer>();
		auto& ui_sprite = ui_ele.GetComponent<SpriteRenderer>();
		ui_sprite.rgba = { 1.f,1.f,1.f,1.f };
		ui_sprite.textureHandle = (GUID)DefaultResourceIDs::COLOR_DEADED_DEFAULT;

		return ui_ele;
	}
	GameObject GOFactory::CreateGO_Button()
	{
		auto ui_ele = CreateGO("Button");
		ui_ele.AddComponent<RectTransform>();
		auto& ui_rect = ui_ele.GetComponent<RectTransform>();
		ui_rect.width = 100; ui_rect.height = 100; ui_rect.pos_x = 0; ui_rect.pos_y = 0;

		ui_ele.AddComponent<SpriteRenderer>();
		ui_ele.AddComponent<Button>();

		return ui_ele;
	}
	GameObject GOFactory::CreateGO_Slider()
	{
		auto ui_ele = CreateGO("Slider");
		ui_ele.AddComponent<RectTransform>();
		auto& ui_rect = ui_ele.GetComponent<RectTransform>();
		ui_rect.width = 200; ui_rect.height = 50; ui_rect.pos_x = 0; ui_rect.pos_y = 0;
		ui_rect.final_width = 200.f;	//helps with init slider

		ui_ele.AddComponent<SpriteRenderer>();
		ui_ele.AddComponent<Slider>();

		GameObject fill = CreateGO_Image();
		fill.SetName("fill");
		SetParent(fill.GetEntity(), ui_ele.GetEntity());
		auto& fill_image = fill.GetComponent<SpriteRenderer>();
		fill_image.rgba = { 1.f,1.f,1.f,1.f };
		fill_image.raycast_target = false;

		GameObject handle = CreateGO_Image();
		handle.SetName("handle");
		SetParent(handle.GetEntity(), ui_ele.GetEntity());
		auto& handle_image = handle.GetComponent<SpriteRenderer>();
		handle_image.rgba = { 0.f,0.f,1.f,1.f };
		handle_image.raycast_target = false;
		auto& handle_rect = handle.GetComponent<RectTransform>();
		handle_rect.width = 50;
		handle_rect.height = 50;

		auto& slider = ui_ele.GetComponent<Slider>();
		slider.handle = handle.GetEntity();
		slider.fill = fill.GetEntity();
		slider.SetValue(0.f, ui_ele.GetEntity());

		return ui_ele;
	}

	GameObject GOFactory::CreateGO_Text()
	{
		auto ui_ele = CreateGO("Text");
		ui_ele.AddComponent<RectTransform>();
		auto& ui_rect = ui_ele.GetComponent<RectTransform>();
		ui_rect.width = 100; ui_rect.height = 100; ui_rect.pos_x = 0; ui_rect.pos_y = 0;
		ui_ele.AddComponent<FontRenderer>();
		auto& ui_font = ui_ele.GetComponent<FontRenderer>();
		ui_font.rgba = { 0.f,0.f,0.f,1.f };
		ui_font.font_size = 50;
		ui_font.line_spacing = 1.25f;
		ui_font.fontHandle = (GUID)DefaultResourceIDs::FONT_BLANK_DEFAULT;

		return ui_ele;
	}


	GameObject GOFactory::CreateGO_Model(GUID skele_guid, GUID anim_guid, GUID model_guid) {
		//Get the resource handle first
		auto& model = *Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>(model_guid).get();
		int skele_index = 0;
		return CreateGO_ModelNode(model.rootNode, skele_guid, anim_guid, model_guid, entt::null, entt::null, skele_index, model.is_static);
	}

	GameObject GOFactory::CreateGO_ModelNode(SliceEngineTypes::ModelNode const& node, GUID skele_guid, GUID anim_guid, GUID model_guid, Entity parent, Entity root, int& index, bool is_static) {
		auto go = CreateGO(node.name);
		SetParent(go.GetEntity(), parent);

		auto rm = Core::GetInstance()->GetResourceManager();
		//if its the start of the tree, set it as the root
		

		if (!node.mesh_ref.empty()) {
			go.AddComponent<Renderer>();
			auto& rc = go.GetComponent<Renderer>();
			rc.modelHandle = rm->get<SliceEngineTypes::Model>(model_guid);
			rc.meshOffset = static_cast<unsigned char>(node.mesh_ref[0]);

			if (!is_static)
				rc.skinned = true;
			//rc.texture = (GUID)18349208178533231704;

			//add siblings if a single node has multiple mesh refs
			for (int i = 1; i < node.mesh_ref.size(); ++i) {
				auto sibling = CreateGO(node.name);
				SetParent(sibling.GetEntity(), parent);

				auto& s_tform = sibling.GetComponent<Transform>();
				s_tform.position = node.position;
				s_tform.rotation = node.rotation;
				s_tform.scale = node.scale;

				sibling.AddComponent<Renderer>();
				auto& s_rc = sibling.GetComponent<Renderer>(); 
				s_rc.modelHandle = rm->get<SliceEngineTypes::Model>(model_guid);
				s_rc.meshOffset = static_cast<unsigned char>(node.mesh_ref[i]);

				if (!is_static) {
					Bone tmpSibling;
					tmpSibling.skeleton_root = root;
					tmpSibling.frame_idx = index;

					sibling.AddComponent<Bone>(tmpSibling);
					//auto& s_bone = sibling.GetComponent<Bone>();
					//s_bone.skeleton_root = root;
					//s_bone.frame_idx = index;
					s_rc.skinned = true;
				}
				//rc.texture = (GUID)18349208178533231704;
			}
		}

		if (!is_static)
		{
			Bone tmpBone;
			tmpBone.skeleton_root = root;
			tmpBone.frame_idx = index;

			go.AddComponent<Bone>(tmpBone);
			//auto& bone = go.GetComponent<Bone>();
			//bone.skeleton_root = root;
			//bone.frame_idx = index;



			if (root == entt::null) {
				root = go.GetEntity();
				go.AddComponent<Animator>();
				auto& animator = go.GetComponent<Animator>();
				go.RemoveComponent<Bone>();

				GUID skeletonGUID = skele_guid;
				GUID animPkgGUID = anim_guid;
				if (skeletonGUID != GUID::null())
					animator.Handle_skeleton = Core::GetInstance()->GetResourceManager()->get<SliceEngine::SliceEngineTypes::Skeleton>(skeletonGUID);
				if (animPkgGUID != GUID::null())
				{
					animator.Handle_curr_anim_pkg = Core::GetInstance()->GetResourceManager()->get<SliceEngine::SliceEngineTypes::AnimationPackage>(animPkgGUID);
					animator.curr_anim_pkg = *animator.Handle_curr_anim_pkg.get();

				}
				if (animator.Handle_stateMachine.IsValid())
				{
					animator.stateMachine.EFSM = *animator.Handle_stateMachine.get();
					animator.stateMachine.InitState(animator.curr_anim_pkg);
				}
			}
		}

		for (auto& child : node.children) {
			CreateGO_ModelNode(child, skele_guid, anim_guid, model_guid, go.GetEntity(), root, ++index, is_static);
		}
		//set node local tform here, since setparent does some calculations to decompose relative mtx
		//infact, do it after recursion, so everything has default values
		auto& tform = go.GetComponent<Transform>();
		tform.position = node.position;
		tform.rotation = node.rotation;
		tform.scale = node.scale;

		return go;
	}

	void GOFactory::DebugPrint()
	{
		// map size
		//std::cout << "Total GameObjects: " << mEntityToGO.size() << std::endl;
		// name map size
		//std::cout << "Total Names: " << mNameToEntity.size() << std::endl;

		auto entityView = mRegistry.view<SliceEntity>();
		for (auto entity : entityView)
		{
			//std::cout << (uint32_t)entity << " : " << mEntityToGO[entity].GetName() << std::endl;
		}
	}

	void GOFactory::TestLoop()
	{
		auto entityView = mRegistry.view<SliceEntity>();
		for (auto entity : entityView)
		{
			//std::cout << mEntityToGO[entity].GetName() << std::endl;
			
			
			for (auto&& [type_id, storage] : mRegistry.storage())
			{
				if (storage.contains(entity))
				{
					// each component will be here
					//std::cout << storage.type().name() << std::endl;
					
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
					if (value.is_type<int>());
					//std::cout << property.get_name() << " = " << value.get_value<int>() << std::endl;
					else if (value.is_type<float>());
					//std::cout << property.get_name() << " = " << value.get_value<float>() << std::endl;
					else if (value.is_type<double>());
						//std::cout << property.get_name() << " = " << value.get_value<double>() << std::endl;
					else if (value.get_type() == rttr::type::get<uint64_t>() ||
						value.get_type().is_derived_from(rttr::type::get<uint64_t>()))
					{
						//std::cout << property.get_name() << " = " << value.get_value<uint64_t>() << std::endl;
					}
					else if (value.is_type<std::array<Entity, 4>>())
					{
						auto arr = value.get_value<std::array<Entity, 4>>();
						//std::cout << property.get_name() << " = [";
						for (size_t i = 0; i < arr.size(); ++i);
							//std::cout << static_cast<uint32_t>(arr[i]) << (i + 1 < arr.size() ? ", " : "");
						//std::cout << "]" << std::endl;
					}
					else if (value.is_type<glm::vec3>())
					{
						glm::vec3 v = value.get_value<glm::vec3>();
						//std::cout << property.get_name() << " = (" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
					}
					else if (value.get_type() == rttr::type::get<uint32_t>() ||
						value.get_type().is_derived_from(rttr::type::get<uint32_t>()))
					{
						uint32_t u = value.get_value<uint32_t>();
						//std::cout << property.get_name() << " = " << u << std::endl;
					}
					else if (value.is_type<std::string>() ||
						value.get_type().is_derived_from(rttr::type::get<std::string>()))
					{
						std::string str = value.get_value<std::string>();
						//std::cout << property.get_name() << " = \"" << str << "\"" << std::endl;
					}
					else
					{
						//std::cout << property.get_name() << " = <unsupported type>" << std::endl;
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

			// if the old name wasnt in the map, means this entity's name doesnt belong in the map
			if (mRegistry.any_of<PrefabEditingEntity>(entity))
			{
				return;
			}

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


			
			// not the best way to check for prefab editing entity
			// but this the fastest way i can think of rn
			if (!mEntityToGO[Entity].HasComponent<PrefabEditingEntity>())
				mNameToEntity.erase(mEntityToGO[Entity].GetName());

			////std::cout << "Destryoing entity : " << (uint32_t)Entity << std::endl;
			// idk if its okay to destroy EnTT entity before clearing from map
			// but ill leave it like this for now
			mEntityToGO[Entity].Destroy();

			// erase from the maps
			mEntityToGO.erase(Entity);

			//mRegistry.destroy(Entity);
		}

		mDeleteList.clear();
	}

	void GOFactory::SceneGraphDelete(Entity entity)
	{
		auto it = mEntityToGO.find(entity);

		if (it == mEntityToGO.end())
		{
			SLICE_LOG("Entity already deleted!");
			return;
		}
		auto& sceneGraph = mEntityToGO[entity].GetComponent<SceneGraph>();
		//Check for siblings
		if (sceneGraph.neighbours[SceneGraph::LEFT] != entt::null && sceneGraph.neighbours[SceneGraph::RIGHT] != entt::null)
		{
			//Check if the left/right neighbours exist in the mEntityToGO(For changing scenes)
			auto iter = mEntityToGO.find(sceneGraph.neighbours[SceneGraph::LEFT]);
			auto iter2 = mEntityToGO.find(sceneGraph.neighbours[SceneGraph::RIGHT]);

			//if (it == mEntityToGO.end() || it2 == mEntityToGO.end()) //One of the neighbours have been deleted alr
			//{

			//}
			if (iter != mEntityToGO.end() && iter2 != mEntityToGO.end() && (mEntityToGO[sceneGraph.neighbours[SceneGraph::LEFT]].HasComponent<SceneGraph>() && mEntityToGO[sceneGraph.neighbours[SceneGraph::RIGHT]].HasComponent<SceneGraph>()))
			{
				auto& leftSiblingGraph = mEntityToGO[sceneGraph.neighbours[SceneGraph::LEFT]].GetComponent<SceneGraph>();
				auto& rightSiblingGraph = mEntityToGO[sceneGraph.neighbours[SceneGraph::RIGHT]].GetComponent<SceneGraph>();

				leftSiblingGraph.neighbours[SceneGraph::RIGHT] = sceneGraph.neighbours[SceneGraph::RIGHT];
				rightSiblingGraph.neighbours[SceneGraph::LEFT] = sceneGraph.neighbours[SceneGraph::LEFT];
			}
		}
		else if (sceneGraph.neighbours[SceneGraph::LEFT] != entt::null)
		{
			auto iter = mEntityToGO.find(sceneGraph.neighbours[SceneGraph::LEFT]);
			//if (it == mEntityToGO.end()) //One of the neighbours have been deleted alr
			//{
			//	//wait idk what to do here tho
			//}
			if (iter != mEntityToGO.end() && mEntityToGO[sceneGraph.neighbours[SceneGraph::LEFT]].HasComponent<SceneGraph>())
			{
				auto& leftSiblingGraph = mEntityToGO[sceneGraph.neighbours[SceneGraph::LEFT]].GetComponent<SceneGraph>();
				leftSiblingGraph.neighbours[SceneGraph::RIGHT] = entt::null;
			}
		}
		else if (sceneGraph.neighbours[SceneGraph::RIGHT] != entt::null)
		{
			auto iter = mEntityToGO.find(sceneGraph.neighbours[SceneGraph::RIGHT]);
			//if (it == mEntityToGO.end()) //One of the neighbours have been deleted alr
			//{
			//	//wait idk what to do here tho
			//}
			if (iter != mEntityToGO.end() && mEntityToGO[sceneGraph.neighbours[SceneGraph::RIGHT]].HasComponent<SceneGraph>())
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
				if (parentGraph.neighbours[SceneGraph::DOWN] == entity)
				{
					parentGraph.neighbours[SceneGraph::DOWN] = sceneGraph.neighbours[SceneGraph::RIGHT];
				}
			}
			else if (mEntityToGO.find(sceneGraph.neighbours[SceneGraph::UP]) != mEntityToGO.end())
			{
				if (mEntityToGO[sceneGraph.neighbours[SceneGraph::UP]].HasComponent<SceneGraph>())
				{
					auto& parentGraph = mEntityToGO[sceneGraph.neighbours[SceneGraph::UP]].GetComponent<SceneGraph>();

					if (parentGraph.neighbours[SceneGraph::DOWN] == entity)
					{
						parentGraph.neighbours[SceneGraph::DOWN] = sceneGraph.neighbours[SceneGraph::RIGHT];
					}
				}
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
			std::string componentName(storage.type().name());

			rttr::type componentType = rttr::type::get_by_name(componentName);
			if (!componentType)
			{
				//SLICE_LOG_ERROR("Component is not registered");
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