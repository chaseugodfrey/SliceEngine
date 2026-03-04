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
	//I'm not sure if this is really necessary
	bool BoolMultipleSelection(SelectionManager* selectionManager, bool currentSelection, bool isMultiSelection, std::function<bool(Entity)> func)
	{
		if (isMultiSelection)
		{
			//Do the difference check (this one is for tags)
			for (auto selectedNode : selectionManager->GetSelectedNodes())
			{
				if (selectedNode->type == SelectionType::ENTITY)
				{
					Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;
					bool currentBool = func(currentEntity);

					if (currentBool != currentSelection)
					{
						return true;
					}
				}
			}
		}
		return false;
	}
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
	std::array<bool,3> Vector3MultipleSelection(SelectionManager* selectionManager, glm::vec3 currentSelection, bool isMultiSelection, std::function<glm::vec3(Entity)> func)
	{
		std::array<bool,3> isSelectionDifferent = std::array<bool, 3>{ false,false,false };
		if (isMultiSelection)
		{
			//Do the difference check (this one is for tags)
			for (auto selectedNode : selectionManager->GetSelectedNodes())
			{
				if (selectedNode->type == SelectionType::ENTITY)
				{
					Entity currentEntity = static_cast<EntityNode*>(selectedNode)->entity;

					glm::vec3 currentVec3 = func(currentEntity);

					if (currentVec3.x != currentSelection.x)
					{
						isSelectionDifferent[0] = true;
					}

					if (currentVec3.y != currentSelection.y)
					{
						isSelectionDifferent[1] = true;
					}

					if (currentVec3.z != currentSelection.z)
					{
						isSelectionDifferent[2] = true;
					}
				}
			}
		}
		return isSelectionDifferent;
	}
}