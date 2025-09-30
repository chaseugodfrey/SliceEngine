#include "pch.h"
#include "SelectionSystem.h"

namespace SliceEditor
{
	void SelectionSystem::UpdateManagers()
	{
		for (size_t i = 0; i < mListeners.size(); i++)
		{
			mListeners[i]->OnUpdateSelected(mSelectedEntities);
		}
	}

	void SelectionSystem::RegisterListener(ISelectionListener* listener)
	{
		mListeners.push_back(listener);
	}

	void SelectionSystem::UpdateSelected(entt::entity entity)
	{
		auto it = std::find(std::begin(mSelectedEntities), std::end(mSelectedEntities), entity);
		if (it == std::end(mSelectedEntities))
			mSelectedEntities.insert(entity);

		std::unordered_set<entt::entity> set{ entity };
		for (auto& listener : mListeners)
		{
			listener->OnUpdateSelected(set);
		}
	}

	void SelectionSystem::UpdateDeslected(entt::entity entity)
	{
		auto it = std::find(std::begin(mSelectedEntities), std::end(mSelectedEntities), entity);
		if (it != std::end(mSelectedEntities))
			mSelectedEntities.erase(it);

		std::unordered_set<entt::entity> set{ entity };
		for (auto& listener : mListeners)
		{
			listener->OnUpdateDeselected(set);
		}
	}


	void SelectionSystem::UpdateSelected(std::unordered_set<entt::entity>& entities)
	{
		std::unordered_set<entt::entity> selected{};
		std::unordered_set<entt::entity> deselected{};

		for (auto entity : entities)
		{
			auto it = std::find(std::begin(mSelectedEntities), std::end(mSelectedEntities), entity);
			if (it == std::end(mSelectedEntities))
				selected.insert(entity);
			else
				deselected.insert(entity);
		}

		// to do: get managers to subscribe
		mSelectedEntities = entities;

		// update all listeners
		for (auto& listener : mListeners)
		{
			listener->OnUpdateSelected(selected);
			listener->OnUpdateDeselected(deselected);
		}
	}

	void SelectionSystem::UpdateDeslected(std::unordered_set<entt::entity>& entities)
	{

	}

	void SelectionSystem::ClearSelection()
	{
		for (auto& listener : mListeners)
		{
			listener->OnUpdateDeselected(mSelectedEntities);
		}

		// to do: get managers to subscribe
		mSelectedEntities.clear();
	}

	std::unordered_set<entt::entity>& SelectionSystem::GetSelectedEntities()
	{
		return mSelectedEntities;
	}
}
