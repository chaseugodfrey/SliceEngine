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
	void SelectionManager::UpdateManagers()
	{
		for (size_t i = 0; i < mListeners.size(); i++)
		{
			mListeners[i]->OnUpdateSelected(mSelectedEntities);
		}
	}

	void SelectionManager::Init()
	{
		mSelectedEntities.clear();
		EventManager::GetInstance()->Subscribe<ClearSelectionEvent, &SelectionManager::ClearSelectionEventHandler>(this);
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

		if (node->type == SelectionNode::SelectionType::ENTITY)
			mSelectionType = SelectionType::ENTITY;

		if (!suppressHistory)
		{
			registry.GetManager<HistoryManager>("History")->AddCommand(std::make_unique<SelectNodeCommand>(*this, oldSelection, mSelectedNodes));
		}

		//std::unordered_set<entt::entity> set{ entity };
		//for (auto& listener : mListeners)
		//{
		//	listener->OnUpdateSelected(set);
		//}
	}

	void SelectionManager::SelectSingle(entt::entity entity, bool suppressHistory)
	{
		auto session = registry.GetManager<SessionManager>("Session");
		auto& node = session->GetEntityNodes().at(entity);
		SelectSingle(node.get(), suppressHistory);
	}

	void SelectionManager::SelectSingleAdd(entt::entity entity, bool suppressHistory)
	{
		auto it = std::find(std::begin(mSelectedEntities), std::end(mSelectedEntities), entity);
		if (it == std::end(mSelectedEntities))
		{
			//if (!suppressHistory)
			//	registry.GetManager<HistoryManager>("History")->AddCommand(std::make_unique<SelectEntityCommand>(*this, mSelectedEntities));

			mSelectedEntities.insert(entity);
		}

		std::unordered_set<entt::entity> set{ entity };
		for (auto& listener : mListeners)
		{
			listener->OnUpdateSelected(set);
		}
	}

	void SelectionManager::UpdateDeslected(entt::entity entity, bool suppressHistory)
	{
		auto it = std::find(std::begin(mSelectedEntities), std::end(mSelectedEntities), entity);
		if (it != std::end(mSelectedEntities))
		{
			//if (!suppressHistory)
			//	registry.GetManager<HistoryManager>("History")->AddCommand(std::make_unique<SelectEntityCommand>(*this, mSelectedEntities));

			mSelectedEntities.erase(it);
		}

		std::unordered_set<entt::entity> set{ entity };
		for (auto& listener : mListeners)
		{
			listener->OnUpdateDeselected(set);
		}
	}

	void SelectionManager::SelectMultiple(std::unordered_set<SelectionNode*> selectedNodes, bool suppressHistory)
	{
		if (!suppressHistory)
			registry.GetManager<HistoryManager>("History")->AddCommand(std::make_unique<SelectNodeCommand>(*this, mSelectedNodes, selectedNodes));

		ClearSelection(suppressHistory);

		mSelectedNodes = selectedNodes;

		for (auto& node : mSelectedNodes)
			node->isSelected = true;
	}

	void SelectionManager::UpdateDeslected(std::unordered_set<entt::entity>& entities, bool suppressHistory)
	{

	}

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
			node->isSelected = false;

		mSelectedNodes.clear();
	}

	std::unordered_set<entt::entity>& SelectionManager::GetSelectedEntities()
	{
		return mSelectedEntities;
	}
	std::unordered_set<SelectionNode*>& SelectionManager::GetSelectedNodes()
	{
		return mSelectedNodes;
	}
}