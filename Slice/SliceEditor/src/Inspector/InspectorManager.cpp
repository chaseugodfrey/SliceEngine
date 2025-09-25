#include <pch.h>
#include "InspectorManager.h"
#include "InspectorWindow.h"

namespace SliceEditor
{
	void InspectorManager::Init()
	{
		
	}

	std::unique_ptr<EditorWindow> InspectorManager::CreateWindow()
	{
		auto window = std::make_unique<InspectorWindow>(*this);
		return window;
	}

}