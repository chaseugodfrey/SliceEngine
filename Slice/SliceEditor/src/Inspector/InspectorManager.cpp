/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        InspectorManager.cpp

 author:	  Chase Rodgrigues
 co-author:   Nic Lai

 email:       rodrigues.i@digipen.edu

 brief:		  Defines the InspectorManager class, which manages the data of the inspector window of the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include <pch.h>
#include "InspectorManager.h"
#include "InspectorWindow.h"
#include "../Core/Registry.h"

namespace SliceEditor
{
	void InspectorManager::Init()
	{
		
	}

	std::unordered_set<entt::entity>& InspectorManager::GetSelectedEntities()
	{
		return registry.GetSelectionSystem().GetSelectedEntities();
	}

	std::unique_ptr<EditorWindow> InspectorManager::CreateEditorWindow()
	{
		auto window = std::make_unique<InspectorWindow>(*this);
		return window;
	}

}