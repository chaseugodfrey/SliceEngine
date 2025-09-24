#ifndef SELECTION_SYSTEM_H
#define SELECTION_SYSTEM_H

#include "ISelectionService.h"

namespace SliceEditor
{
	class SelectionSystem
	{

		std::vector<ISelectionListener*> mListeners;
		std::unordered_set<entt::entity> mSelectedEntities;
		
		
		void UpdateManagers();


	public:
		SelectionSystem() = default;
		~SelectionSystem() = default;

		void RegisterListener(ISelectionListener* listener);
	};
}

#endif