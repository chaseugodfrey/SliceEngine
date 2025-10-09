/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        HierarchyManager.cpp

 author:	  Chase Rodgrigues
 co-author:   Nic Lai

 email:       rodrigues.i@digipen.edu

 brief:		  Defines the HierarchyManager class, which is responsible for managing the hierarchy of game objects in the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include <pch.h>
#include "HierarchyManager.h"
#include "HierarchyWindow.h"
#include "../Core/Registry.h"
#include "../Selection/SelectionManager.h"
#include "../../SliceEngine/src/Systems/SceneSystem.h"
#include "../../SliceEngine/src/Core/EventManager.h"
#include "../../SliceEngine/src/Core/ComponentEventHandler.h"

namespace SliceEditor
{

	void HierarchyManager::Init()
	{
		EventManager::GetInstance()->Subscribe<OnSceneLoadedEvent, &HierarchyManager::OnSceneLoad>(this);
		BuildHierarchy();
	}

	void HierarchyManager::Reset()
	{
	}

	void HierarchyManager::OnSceneLoad(OnSceneLoadedEvent& event)
	{
		if (event.isSceneLoaded)
		{
			registry.GetManager<SelectionManager>("Selection")->ClearSelection(true);
		}
	}

	void HierarchyManager::SetDirty()
	{
		isDirty = true;
	}

	void HierarchyManager::CheckDirty()
	{
		if (isDirty)
		{
			BuildHierarchy();
			isDirty = false;
		}
	}

	void HierarchyManager::AddEntityDirectly(entt::entity entity)
	{
		TestNode node{};
		node.entity = entity;
		mHierarchy.emplace(entity, node);
	}

	void HierarchyManager::BuildHierarchy()
	{
		mHierarchy.clear();

		auto core = SliceEngine::Core::GetInstance();
		auto& reg = core->GetRegistry();
		auto root_entity = core->mFactory.GetRootEntity();

		TestNode rootNode{};
		rootNode.entity = root_entity;
		rootNode.name = core->GetSceneSystem()->GetCurrentScenePath().filename().stem().string();
		mHierarchy.emplace(root_entity, rootNode);

		auto hierarchy = reg.view<SliceEngine::SliceEntity>();

		for (auto entity : hierarchy)
		{
			TestNode node{};
			node.entity = entity;
			//SLICE_LOG_VALUES("Adding entity to hierarchy: " + reg.get<SliceEngine::SliceEntity>(entity).mName);
			mHierarchy.emplace(entity, node);
		}
	}

	TestNode& HierarchyManager::GetSceneRootNode()
	{
		auto entity = SliceEngine::FactoryInstance.GetRootEntity();
		return mHierarchy[entity];
	}

	void HierarchyManager::AddGameObject()
	{
		auto& factory = SliceEngine::Core::GetInstance()->mFactory;
		auto go = factory.CreateGO();

		// todo : remove
		go.AddComponent<SliceEngine::Renderer>();

		TestNode node{};
		node.entity = go.GetEntity();
		mHierarchy.emplace(node.entity, node);


		auto rootEntity = factory.GetRootEntity();
		ParentGameObject(node.entity, rootEntity);

		//isDirty = true;
		SliceEngine::OnGONetworkEvent(go.GetEntity(),true);
	}

	void HierarchyManager::RemoveGameObject(entt::entity target)
	{
		// remove everything from selection system(temp fix)
		registry.GetManager<SelectionManager>("Selection")->ClearSelection();

		// remove from core registry
		SliceEngine::FactoryInstance.Destroy(target);
		SetDirty();
	}

	void HierarchyManager::ParentGameObject(entt::entity child_entity, entt::entity parent_entity)
	{
		// set parent in engine
		auto& factory = SliceEngine::Core::GetInstance()->mFactory;
		auto go = factory.GetGOByEntity(child_entity);
		factory.SetParent(child_entity, parent_entity);
	}

	void HierarchyManager::Unparent(entt::entity child)
	{
		auto& factory = SliceEngine::Core::GetInstance()->mFactory;
		factory.Unparent(child);
	}

	void HierarchyManager::SetNewLocation(entt::entity target, entt::entity destination)
	{
		auto& factory = SliceEngine::Core::GetInstance()->mFactory;
		//If its the same entity, do nothing
		if(destination == target)
		{
			SLICE_LOG_DEBUG("Trying to set new location to same entity, doing nothing");
			return;
		}
		//It be moving somewhere else
		else
		{
			auto& destSceneGraph = factory.GetGOByEntity(destination).GetComponent<SliceEngine::SceneGraph>();
			factory.SetNewSceneGraphLocation(target, destination, destSceneGraph.neighbours[SliceEngine::SceneGraph::LEFT]);
		}

		SetDirty();
	}

	std::unique_ptr<EditorWindow> HierarchyManager::CreateEditorWindow()
	{
		auto window = std::make_unique<HierarchyWindow>(*this);
		return window;
	}

	void HierarchyManager::OnUpdateSelected(std::unordered_set<entt::entity>& selected_entities)
	{
		for (auto entity : selected_entities)
		{
			auto it = mHierarchy.find(entity);
			if (it != std::end(mHierarchy))
			{
				it->second.isSelected = true;
			}
		}
	}

	void HierarchyManager::OnUpdateDeselected(std::unordered_set<entt::entity>& deselected_entities)
	{
		for (auto entity : deselected_entities)
		{
			auto it = mHierarchy.find(entity);
			if (it != std::end(mHierarchy))
			{
				it->second.isSelected = false;
			}
		}
	}
}