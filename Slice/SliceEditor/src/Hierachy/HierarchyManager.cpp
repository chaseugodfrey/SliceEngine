#include <pch.h>
#include "HierarchyManager.h"
#include "HierarchyWindow.h"
#include "../Core/Registry.h"

namespace SliceEditor
{

	void HierarchyManager::Init()
	{
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

		auto& scene_graph_comp = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SceneGraph>(entity);
		auto parent_entity = scene_graph_comp.neighbours[SliceEngine::SceneGraph::UP];

		node.parent = &mHierarchy.at(parent_entity);
		node.parent->children.push_back(entity);
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
			mHierarchy.emplace(entity, node);
		}

		for (auto& [entity, node] : mHierarchy)
		{
			auto& scene_graph = reg.get<SliceEngine::SceneGraph>(entity);
			auto parent = scene_graph.neighbours[SliceEngine::SceneGraph::UP];
			auto child = scene_graph.neighbours[SliceEngine::SceneGraph::DOWN];

			if (parent != entt::null)
				node.parent = &mHierarchy[parent];

			while (child != entt::null)
			{
				node.children.push_back(child);
				child = scene_graph.neighbours[SliceEngine::SceneGraph::RIGHT];
			}
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
	}

	void HierarchyManager::RemoveGameObject(entt::entity target)
	{
		//Check for children and remove them first
		auto& targetNode = mHierarchy[target];
		if(targetNode.children.size() > 0)
		{
			for (auto child : targetNode.children)
			{
				RemoveGameObject(child);
			}
		}

		auto& parent_entity = mHierarchy[target].parent;

		// remove from parent's children list
		auto& children = parent_entity->children;
		auto it = std::find(std::begin(children), std::end(children), target);
		children.erase(it);

		// remove from selection system
		registry.GetSelectionSystem().UpdateDeslected({ target });

		// remove from node structure
		mHierarchy.erase(target);

		//Remove from SelectionSystem
		auto& selectedEntities = registry.GetSelectionSystem().GetSelectedEntities();
		auto selectedIt = selectedEntities.find(target);
		if(selectedIt != selectedEntities.end())
		{
			selectedEntities.erase(selectedIt);
		}

		// remove from core registry
		SliceEngine::FactoryInstance.Destroy(target);
	}

	void HierarchyManager::ParentGameObject(entt::entity child_entity, entt::entity parent_entity)
	{
		auto& child_node = mHierarchy.at(child_entity);
		auto& parent_node = mHierarchy.at(parent_entity);
		// detach from previous parent
		// skip if node was just created
		if (child_node.parent != nullptr)
		{
			auto& children_list = child_node.parent->children;

			auto it = std::find(std::begin(children_list), std::end(children_list), child_node.entity);
			if (it != children_list.end())
			{
				children_list.erase(it);
			}
		}

		// add new parent
		child_node.parent = &parent_node;

		// attach to new parent
		parent_node.children.push_back(child_node.entity);
	}

	void HierarchyManager::Unparent(entt::entity child, entt::entity parent)
	{
		auto& childNode = mHierarchy[child];
		auto& parentNode = mHierarchy[parent];
		auto& grandParentNode = mHierarchy[parentNode.parent->entity];
		
		auto& factory = SliceEngine::Core::GetInstance()->mFactory;
		auto go = factory.GetGOByEntity(childNode.entity);
		factory.SetParent(childNode.entity, grandParentNode.entity);
		ParentGameObject(childNode.entity, grandParentNode.entity);
	}

	std::unique_ptr<EditorWindow> HierarchyManager::CreateWindow()
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