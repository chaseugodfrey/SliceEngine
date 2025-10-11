/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        ProfilerWindow.h

 author:	  Nic Lai

 email:       n.lai@digipen.edu

 brief:		  Declares the ProfilerWindow class, which is the window that draws the profiler data of the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef PROFILER_WINDOW_H
#define PROFILER_WINDOW_H

#include "../WindowManager/EditorWindow.h"
//#include "ProfilerManager.h"

namespace SliceEditor
{
	class ProfilerManager;
	class SelectionManager;

	class ProfilerWindow : public EditorWindow
	{
		ProfilerManager& mManager;

	public:
		~ProfilerWindow() = default;

		ProfilerWindow(ProfilerManager& manager);

		void Draw() override final;

		void DrawLoggerTab();

		void DrawPerformanceTab();

		void DrawSceneGraphTab();

		void DrawSystemTimeline();

		void DrawSystemBreakdown();

		void DrawSceneGraphComponent(entt::entity entity);
	};
}

#endif