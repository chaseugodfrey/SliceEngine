/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        SelectionSystem.h

 author:	  Chase Rodgrigues

 email:       rodrigues.i@digipen.edu

 brief:		  Declares the SelectionSystem class, which manages the selection of entities in the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

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