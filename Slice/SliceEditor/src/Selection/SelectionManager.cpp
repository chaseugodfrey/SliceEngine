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

	void SelectionManager::RegisterListener(ISelectionListener* listener)
	{
		mListeners.push_back(listener);
	}

	void SelectionManager::SelectSingle(entt::entity entity, bool suppressHistory)
	{
		std::unordered_set<entt::entity> oldSelection = mSelectedEntities;

		ClearSelection(suppressHistory);
		mSelectedEntities.insert(entity);

		if (!suppressHistory)
		{
			registry.GetManager<HistoryManager>("History")->AddCommand(std::make_unique<SelectEntityCommand>(*this, oldSelection, mSelectedEntities));
		}

		std::unordered_set<entt::entity> set{ entity };
		for (auto& listener : mListeners)
		{
			listener->OnUpdateSelected(set);
		}
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

	void SelectionManager::SelectMultiple(std::unordered_set<entt::entity>& entities, bool suppressHistory)
	{
		std::unordered_set<entt::entity> selected{};
		std::unordered_set<entt::entity> deselected{};

		//for (auto entity : entities)
		//{
		//	auto it = std::find(std::begin(mSelectedEntities), std::end(mSelectedEntities), entity);
		//	if (it == std::end(mSelectedEntities))
		//		selected.insert(entity);
		//	else
		//		deselected.insert(entity);
		//}

		mSelectedEntities = entities;

		// update all listeners
		for (auto& listener : mListeners)
		{
			listener->OnUpdateSelected(selected);
			listener->OnUpdateDeselected(deselected);
		}
	}

	void SelectionManager::UpdateDeslected(std::unordered_set<entt::entity>& entities, bool suppressHistory)
	{

	}

	void SelectionManager::ClearSelection(bool suppressHistory)
	{
		if (mSelectedEntities.empty())
			return;

		if (!suppressHistory)
			registry.GetManager<HistoryManager>("History")->AddCommand(std::make_unique<SelectEntityCommand>(*this, mSelectedEntities, std::unordered_set<entt::entity>{}));

		for (auto& listener : mListeners)
		{
			listener->OnUpdateDeselected(mSelectedEntities);
		}

		// to do: get managers to subscribe
		mSelectedEntities.clear();
	}

	std::unordered_set<entt::entity>& SelectionManager::GetSelectedEntities()
	{
		return mSelectedEntities;
	}
}