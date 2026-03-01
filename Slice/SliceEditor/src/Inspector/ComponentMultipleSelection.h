/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        ComponentMultipleSelection.h

 author:	  Nic Lai

 email:       n.lai@digipen.edu

 brief:		  Declares the ComponentMultiSelection file, which wraps multi-selection functions to create support for certain multi-selection use cases.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef COMPONENT_MULTIPLE_SELECTION_H
#define COMPONENT_MULTIPLE_SELECTION_H

#include <Selection/SelectionManager.h>

namespace SliceEditor
{
	class Registry;
	class SelectionManager;

	bool StringMultipleSelection(SelectionManager* selectionManager,std::string currentSelection, bool isMultiSelection);

	bool ComboMultipleSelection(SelectionManager* selectionManager, uint32_t currentSelection, bool isMultiSelection);


}

#endif