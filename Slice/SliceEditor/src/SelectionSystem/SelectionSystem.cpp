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

	void SelectionSystem::UpdateSelection(std::unordered_set<entt::entity>& entities)
	{
		mSelectedEntities = entities;
	}
}
