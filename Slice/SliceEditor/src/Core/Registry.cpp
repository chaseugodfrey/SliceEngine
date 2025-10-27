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
#include "Session/SessionManager.h"
#include "Selection/ISelectionListener.h"
#include "Selection/SelectionManager.h"
#include "History/HistoryManager.h"
#include "ContentBrowser/ContentBrowserManager.h"
#include "Profiler/ProfilerManager.h"
#include "WindowManager/WindowManager.h"

namespace SliceEditor
{
	void Registry::Init()
	{
		CreateManager<SessionManager>("Session");
		CreateManager<HistoryManager>("History");
		CreateManager<SelectionManager>("Selection");
		CreateManager<ContentBrowserManager>("ContentBrowser");
		CreateManager<ProfilerManager>("Profiler");
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

	void Registry::Update()
	{
		GetManager<SessionManager>("Session")->Update();
	}
	
	std::unordered_map<std::string, std::unique_ptr<IBaseManager>> const& Registry::GetManagers()
	{
		return mManagers;
	}
}