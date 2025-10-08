/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        Registry.cpp

 author:	  Chase Rodgrigues

 email:       rodrigues.i@digipen.edu

 brief:		  Defines the Registry class, which holds all the managers of the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include <pch.h>
#include "Registry.h"
#include "Selection/ISelectionListener.h"
#include "Selection/SelectionManager.h"
#include "History/HistoryManager.h"
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
		CreateManager<HistoryManager>("History");
		CreateManager<SelectionManager>("Selection");
		CreateManager<ContentBrowserManager>("ContentBrowser");
		CreateManager<HierarchyManager>("Hierarchy");
		CreateManager<InspectorManager>("Inspector");
		CreateManager<SceneViewManager>("SceneView");
		CreateManager<ProfilerManager>("Profiler");
		CreateManager<GameViewManager>("GameView");
		CreateManager<WindowManager>("Windows");


		auto mSelection = GetManager<SelectionManager>("Selection");

		for (auto& [name, manager] : mManagers)
		{
			if (auto listener = dynamic_cast<ISelectionListener*>(manager.get()))
			{
				mSelection->RegisterListener(listener);
			}

			manager->Init();
		}
	}
	
	std::unordered_map<std::string, std::unique_ptr<IBaseManager>> const& Registry::GetManagers()
	{
		return mManagers;
	}
}