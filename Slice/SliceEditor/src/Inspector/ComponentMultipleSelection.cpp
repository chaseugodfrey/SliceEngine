/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        ComponentMultipleSelection.cpp

 author:	  Nic Lai

 email:       n.lai@digipen.edu

 brief:		  Defines the ComponentMultiSelection file, which wraps multi-selection functions to create support for certain multi-selection use cases.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include <pch.h>
#include "ComponentMultipleSelection.h"
#include "Selection/SelectionManager.h"

namespace SliceEditor
{
	bool StringMultipleSelection(SelectionManager* selectionManager, std::string currentSelection, bool isMultiSelection)
	{
		if (isMultiSelection)
		{
			//Do the difference check (this one is for tags)
			for (auto selectedNode : selectionManager->GetSelectedNodes())
			{
				if (selectedNode->type == SelectionType::ENTITY)
				{
					Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;
					std::string currentTag = SliceEngine::FactoryInstance.GetGOByEntity(currentEntity).GetTag();

					if (currentTag != currentSelection)
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	bool ComboMultipleSelection(SelectionManager* selectionManager,uint32_t currentSelection, bool isMultiSelection)
	{
		if (isMultiSelection)
		{
			//Do the difference check (this one is for tags)
			for (auto selectedNode : selectionManager->GetSelectedNodes())
			{
				if (selectedNode->type == SelectionType::ENTITY)
				{
					Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;

					uint32_t currentLayer = SliceEngine::Core::GetInstance()->GetRegistry().get<SliceEngine::SliceEntity>(currentEntity).mLayer;

					if (currentLayer != currentSelection)
					{
						return true;
					}
				}
			}
		}
		return false;
	}
}