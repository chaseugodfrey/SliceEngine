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

	std::unique_ptr<EditorWindow> InspectorManager::CreateWindow()
	{
		auto window = std::make_unique<InspectorWindow>(*this);
		return window;
	}

}