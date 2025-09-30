#ifndef SELECTION_SYSTEM_H
#define SELECTION_SYSTEM_H

#include "ISelectionListener.h"

namespace SliceEditor
{
	class SelectionSystem
	{
		// to do:: make it non-static later
		std::vector<ISelectionListener*> mListeners;
		std::unordered_set<entt::entity> mSelectedEntities;
		
		void UpdateManagers();

	public:
		SelectionSystem() = default;
		~SelectionSystem() = default;

		void RegisterListener(ISelectionListener* listener);
		void UpdateSelected(entt::entity entity);
		void UpdateDeslected(entt::entity entity);
		void UpdateSelected(std::unordered_set<entt::entity>& entities);
		void UpdateDeslected(std::unordered_set<entt::entity>& entities);
		void ClearSelection();

		// replace this with listener pattern
		std::unordered_set<entt::entity>& GetSelectedEntities();
	};
}

#endif