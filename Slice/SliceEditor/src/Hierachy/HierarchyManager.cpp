#include <pch.h>
#include "HierarchyManager.h"
#include "HierarchyWindow.h"
#include "../Core/Registry.h"

namespace SliceEditor
{

	void HierarchyManager::Init()
	{
		//Test();
		BuildHierarchy();
	}

	void HierarchyManager::Reset()
	{
		registry.GetSelectionSystem().ClearSelection();
		BuildHierarchy();
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
			SLICE_LOG_VALUES("Adding entity to hierarchy: " + reg.get<SliceEngine::SliceEntity>(entity).mName);
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
	}

	void HierarchyManager::RemoveGameObject(entt::entity target)
	{
		//Check for children and remove them first
		auto& targetNode = mHierarchy[target];

		// remove everything from selection system(temp fix)
		registry.GetSelectionSystem().ClearSelection();

		// remove from node structure (including children)
		//mHierarchy.erase(target);

		//Remove from SelectionSystem FIX THIS LTR
		/*auto& selectedEntities = registry.GetSelectionSystem().GetSelectedEntities();
		auto selectedIt = selectedEntities.find(target);
		if(selectedIt != selectedEntities.end())
		{
			selectedEntities.erase(selectedIt);
		}*/

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

	void HierarchyManager::SetSiblingIndex(entt::entity target, entt::entity destination)
	{
		auto core = SliceEngine::Core::GetInstance();
		auto& factory = core->mFactory;
		auto go = factory.GetGOByEntity(target);

		auto& dest_scene_graph = core->GetRegistry().get<SliceEngine::SceneGraph>(destination);
		auto& parent_scene_graph = core->GetRegistry().get<SliceEngine::SceneGraph>(dest_scene_graph.neighbours[SliceEngine::SceneGraph::UP]);

		auto child_entity = parent_scene_graph.neighbours[SliceEngine::SceneGraph::DOWN];

		int index{};
		while (child_entity != entt::null)
		{
			if (child_entity == destination)
				break;

			auto& sg = core->GetRegistry().get<SliceEngine::SceneGraph>(child_entity);
			child_entity = sg.neighbours[SliceEngine::SceneGraph::RIGHT];
			++index;
		}

		factory.SetSiblingIndex(target, index);
	}

	void HierarchyManager::SetNewLocation(entt::entity target, entt::entity destination)
	{
		if (destination == entt::null)
			ParentGameObject(target);

		else
		{
			auto& engine_reg = SliceEngine::Core::GetInstance()->GetRegistry();
			auto& target_parent_graph = engine_reg.get<SliceEngine::SceneGraph>(target);
			auto& dest_parent_graph = engine_reg.get<SliceEngine::SceneGraph>(destination);
			auto target_parent = target_parent_graph.neighbours[SliceEngine::SceneGraph::UP];
			auto dest_parent = dest_parent_graph.neighbours[SliceEngine::SceneGraph::UP];

			if (target_parent != dest_parent)
			{
				ParentGameObject(target, dest_parent);
			}

			SetSiblingIndex(target, destination);
		}
	}

	std::unique_ptr<EditorWindow> HierarchyManager::CreateEditorWindow()
	{
		auto& selectionSystem = registry.GetSelectionSystem();
		auto window = std::make_unique<HierarchyWindow>(*this, selectionSystem);
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