#ifndef PROFILER_WINDOW_H
#define PROFILER_WINDOW_H

#include "../WindowManager/EditorWindow.h"
//#include "ProfilerManager.h"

namespace SliceEditor
{
	class ProfilerManager;
	class SelectionSystem;

	class ProfilerWindow : public EditorWindow
	{
		ProfilerManager& mManager;
		SelectionSystem& mSelection;

	public:
		~ProfilerWindow() = default;

		ProfilerWindow(ProfilerManager& manager, SelectionSystem& select);

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