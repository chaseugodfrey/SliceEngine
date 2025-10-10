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
#include "../Selection/SelectionManager.h"

namespace SliceEditor
{
	void InspectorManager::Init()
	{
		RegisterDrawer<SliceEngine::Transform>();
		RegisterDrawer<SliceEngine::Renderer>();
		RegisterDrawer<SliceEngine::AudioSource>();
		RegisterDrawer<SliceEngine::RigidBody>();
		RegisterDrawer<SliceEngine::ColliderShape>();
		RegisterDrawer<SliceEngine::Script>();
		//RegisterDrawer<SliceEngine::Camera>();
	}

	std::unique_ptr<EditorWindow> InspectorManager::CreateEditorWindow()
	{
		auto window = std::make_unique<InspectorWindow>(*this);
		return window;
	}
	
	template<typename ComponentType>
	void InspectorManager::DisplayComponentData(ComponentType& component)
	{
		SLICE_LOG_WARNING("Displaying Component: idk");
		for(auto property : rttr::type::get<ComponentType>().get_properties())
		{
			auto propVar = property.get_value(component);
			if (propVar.is_valid())
			{
				// Here you would add code to display the property using ImGui
				// For example, if the property is a float:
				if (propVar.get_type() == rttr::type::get<float>())
				{
					float value = propVar.to_float();
					if (ImGui::DragFloat(property.get_name().to_string().c_str(), &value, 0.1f))
					{
						property.set_value(component, value);
					}
				}
				// Add more type checks and ImGui widgets as needed
			}
		}
	}
}