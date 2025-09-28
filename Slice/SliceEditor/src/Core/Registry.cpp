#include <pch.h>
#include "Registry.h"
#include "SceneView/SceneViewManager.h"
#include "Hierachy/HierarchyManager.h"
#include "Inspector/InspectorManager.h"
#include "ContentBrowser/ContentBrowserManager.h"
#include "Profiler/ProfilerManager.h"

namespace SliceEditor
{
	void Registry::Init()
	{
		CreateManager<ContentBrowserManager>("ContentBrowser");
		CreateManager<HierarchyManager>("Hierarchy");
		CreateManager<InspectorManager>("Inspector");
		CreateManager<SceneViewManager>("SceneView");
		CreateManager<ProfilerManager>("Profiler");

		for (auto& [name, manager] : mManagers)
		{
			manager->Init();
		}
	}
	
	std::unordered_map<std::string, std::unique_ptr<IBaseManager>> const& Registry::GetManagers()
	{
		return mManagers;
	}

	SelectionSystem& Registry::GetSelectionSystem()
	{
		return selectionSystem;
	}
}