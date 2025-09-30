#include <pch.h>
#include "Registry.h"
#include "SelectionSystem/ISelectionListener.h"
#include "SceneView/SceneViewManager.h"
#include "Hierachy/HierarchyManager.h"
#include "Inspector/InspectorManager.h"
#include "ContentBrowser/ContentBrowserManager.h"
#include "Profiler/ProfilerManager.h"
#include "GameView/GameViewManager.h"
#include "WindowManager/WindowManager.h"

namespace SliceEditor
{
	void Registry::Init()
	{
		CreateManager<ContentBrowserManager>("ContentBrowser");
		CreateManager<HierarchyManager>("Hierarchy");
		CreateManager<InspectorManager>("Inspector");
		CreateManager<SceneViewManager>("SceneView");
		CreateManager<ProfilerManager>("Profiler");
		CreateManager<GameViewManager>("GameView");
		CreateManager<WindowManager>("Windows");

		for (auto& [name, manager] : mManagers)
		{
			if (auto listener = dynamic_cast<ISelectionListener*>(manager.get()))
			{
				selectionSystem.RegisterListener(listener);
			}

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