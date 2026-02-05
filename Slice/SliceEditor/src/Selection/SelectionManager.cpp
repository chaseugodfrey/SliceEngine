/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        SelectionSystem.cpp

 author:	  Chase Rodgrigues

 email:       rodrigues.i@digipen.edu

 brief:		  Defines the SelectionSystem class, which manages the selection of entities in the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "SelectionManager.h"
#include "Core/Registry.h"
#include "Session/SessionManager.h"
#include "History/HistoryManager.h"

namespace SliceEditor
{
	//void SelectionManager::UpdateManagers()
	//{
	//	/*for (size_t i = 0; i < mListeners.size(); i++)
	//	{
	//		mListeners[i]->OnUpdateSelected(mSelectedEntities);
	//	}*/
	//}

	void SelectionManager::Init()
	{
		mSelectedNodes.clear();
		EventManager::GetInstance()->Subscribe<ClearSelectionEvent, &SelectionManager::ClearSelectionEventHandler>(this);
		EventManager::GetInstance()->Subscribe<DeleteSelectedEntities, &SelectionManager::DeleteSelectedObjects>(this);
		EventManager::GetInstance()->Subscribe<CloneSelectedEntities, &SelectionManager::CloneSelectedObjects>(this);

	}

	void SelectionManager::Update()
	{

	}

	void SelectionManager::RegisterListener(ISelectionListener* listener)
	{
		mListeners.push_back(listener);
	}

	void SelectionManager::SelectSingle(SelectionNode* node, bool suppressHistory)
	{
		//std::unordered_set<entt::entity> oldSelection = mSelectedEntities;
		std::unordered_set<SelectionNode*> oldSelection = mSelectedNodes;

		ClearSelection(true);
		//mSelectedEntities.insert(entity);
		mSelectedNodes.insert(node);
		node->isSelected = true;

		if (node->type == SelectionType::ENTITY)
		{
			EntityNode* entNode = static_cast<EntityNode*>(node);
			SliceEngine::Core::GetInstance()->mFactory.GetGOByEntity(entNode->entity).AddComponent<SliceEngine::SelectedEntity>();
		}
		else if (node->type == SelectionType::PREFAB)
		{
			//Get the PrefabGUID for the event of changing Hierarchy and Inspector to Prefab Inspecting
			DirectoryNode* dirNode = static_cast<DirectoryNode*>(node);
			auto& assetManager = registry.GetAssetManager();
			auto rm = SliceEngine::Core::GetInstance()->GetResourceManager();
			SliceEngine::GUID prefabGUID;
			std::string fileName = std::filesystem::relative(dirNode->fullPath.lexically_normal(), registry.GetAssetManager().mAssetDirectory.lexically_normal()).generic_string();
			//Search for the GUID in the map:
			if (assetManager.mFilenameToGUID.find(fileName) != assetManager.mFilenameToGUID.end())
			{
				prefabGUID = assetManager.mFilenameToGUID[fileName];
			}
			else
			{
				SLICE_LOG_CRITICAL("Prefab Inspected not in AssetManager!");
				return;
			}

			PrefabInspectedEvent event(prefabGUID, true);
			EventManager::GetInstance()->Publish<PrefabInspectedEvent>(event);
		}

		mSelectionType = node->type;

		if (!suppressHistory)
		{
			registry.GetManager<HistoryManager>("History")->AddCommand(std::make_unique<SelectNodeCommand>(*this, oldSelection, mSelectedNodes));
		}
	}

	void SelectionManager::SelectSingle(entt::entity entity, bool suppressHistory)
	{
		auto session = registry.GetManager<SessionManager>("Session");
		if (session->IsPrefabInspected())
		{
			auto& node = session->GetPrefabNodes().at(entity);
			SelectSingle(node.get(), suppressHistory);
		}
		else
		{
			auto& node = session->GetEntityNodes().at(entity);
			SelectSingle(node.get(), suppressHistory);
		}
	}

	void SelectionManager::SelectSingleAdd(SelectionNode* node, bool suppressHistory)
	{
		std::unordered_set<SelectionNode*> oldSelection = mSelectedNodes;

		auto it = mSelectedNodes.find(node);
		if (it == std::end(mSelectedNodes))
		{
			node->isSelected = true;
			mSelectedNodes.insert(node);
			if (node->type == SelectionType::ENTITY)
			{
				EntityNode* entNode = static_cast<EntityNode*>(node);
				SliceEngine::Core::GetInstance()->mFactory.GetGOByEntity(entNode->entity).AddComponent<SliceEngine::SelectedEntity>();
			}
		}
		else
		{
			node->isSelected = false;
			mSelectedNodes.erase(it);
			if (node->type == SelectionType::ENTITY)
			{
				EntityNode* entNode = static_cast<EntityNode*>(node);
				auto go = SliceEngine::Core::GetInstance()->mFactory.GetGOByEntity(entNode->entity);
				if (go.HasComponent<SliceEngine::SelectedEntity>())
					go.RemoveComponent<SliceEngine::SelectedEntity>();
			}
		}

		if (!suppressHistory)
			registry.GetManager<HistoryManager>("History")->AddCommand(std::make_unique<SelectNodeCommand>(*this, oldSelection, mSelectedNodes));

	}

	void SelectionManager::SelectSingleAdd(entt::entity entity, bool suppressHistory)
	{
		auto session = registry.GetManager<SessionManager>("Session");
		auto& node = session->GetEntityNodes().at(entity);
		SelectSingleAdd(node.get(), suppressHistory);

	}

	//void SelectionManager::UpdateDeslected(entt::entity entity, bool suppressHistory)
	//{
	//	auto it = std::find(std::begin(mSelectedEntities), std::end(mSelectedEntities), entity);
	//	if (it != std::end(mSelectedEntities))
	//	{
	//		//if (!suppressHistory)
	//		//	registry.GetManager<HistoryManager>("History")->AddCommand(std::make_unique<SelectEntityCommand>(*this, mSelectedEntities));

	//		auto go = SliceEngine::Core::GetInstance()->mFactory.GetGOByEntity(entity);
	//		if (go.HasComponent<SliceEngine::SelectedEntity>())
	//			go.RemoveComponent<SliceEngine::SelectedEntity>();
	//		
	//		mSelectedEntities.erase(it);
	//	}

	//	std::unordered_set<entt::entity> set{ entity };
	//	for (auto& listener : mListeners)
	//	{
	//		listener->OnUpdateDeselected(set);
	//	}
	//}

	void SelectionManager::SelectMultiple(std::unordered_set<SelectionNode*> selectedNodes, bool suppressHistory)
	{
		if (!suppressHistory)
			registry.GetManager<HistoryManager>("History")->AddCommand(std::make_unique<SelectNodeCommand>(*this, mSelectedNodes, selectedNodes));

		ClearSelection(suppressHistory);

		mSelectedNodes = selectedNodes;

		for (auto& node : mSelectedNodes)
		{
			node->isSelected = true;
			if (node->type == SelectionType::ENTITY)
			{
				EntityNode* entNode = static_cast<EntityNode*>(node);
				SliceEngine::Core::GetInstance()->mFactory.GetGOByEntity(entNode->entity).AddComponent<SliceEngine::SelectedEntity>();
			}
		}
	}

	/*void SelectionManager::UpdateDeslected(std::unordered_set<entt::entity>& entities, bool suppressHistory)
	{

	}*/

	void SelectionManager::ClearSelectionEventHandler(ClearSelectionEvent& event)
	{
		ClearSelection(event.suppressHistory);
	}

	void SelectionManager::ClearSelection(bool suppressHistory)
	{
		if (mSelectedNodes.empty())
			return;

		if (!suppressHistory)
			registry.GetManager<HistoryManager>("History")->AddCommand(std::make_unique<SelectNodeCommand>(*this, mSelectedNodes, std::unordered_set<SelectionNode*>{}));

		for (auto& node : mSelectedNodes)
		{
			node->isSelected = false;
			if (node->type == SelectionType::ENTITY)
			{
				EntityNode* entNode = static_cast<EntityNode*>(node);
				auto go = SliceEngine::Core::GetInstance()->mFactory.GetGOByEntity(entNode->entity);
				if (go.HasComponent<SliceEngine::SelectedEntity>())
					go.RemoveComponent<SliceEngine::SelectedEntity>();
			}
		}

		mSelectionType = SelectionType::NONE;
		mSelectedNodes.clear();
	}

	void SelectionManager::DeleteSelectedObjects()
	{
		std::vector<entt::entity> deleteList;
		for (auto* node : mSelectedNodes)
		{
			if (node->type == SelectionType::ENTITY)
			{
				EntityNode& entityNode = *static_cast<EntityNode*>(node);
				deleteList.push_back(entityNode.entity);
			}
		}

		for(auto entity : deleteList)
		{
			EditorUtilities::GameObject_Destroy(entity);
		}
	}

	void SelectionManager::CloneSelectedObjects()
	{
		for (auto* node : mSelectedNodes)
		{
			if (node->type == SelectionType::ENTITY)
			{
				EntityNode& entityNode = *static_cast<EntityNode*>(node);
				EditorUtilities::GameObject_Clone(entityNode.entity);
			}
		}
	}

	/*std::unordered_set<entt::entity>& SelectionManager::GetSelectedEntities()
	{
		return mSelectedEntities;
	}*/
	std::unordered_set<SelectionNode*>& SelectionManager::GetSelectedNodes()
	{
		return mSelectedNodes;
	}
}