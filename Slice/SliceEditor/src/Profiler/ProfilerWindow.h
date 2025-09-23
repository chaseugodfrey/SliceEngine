#ifndef PROFILER_WINDOW_H
#define PROFILER_WINDOW_H

#include "../WindowManager/EditorWindow.h"
#include "ProfilerManager.h"

namespace SliceEditor
{
	class ProfilerWindow : public EditorWindow
	{
		ProfilerManager& manager;
	public:
		~ProfilerWindow() = default;

		ProfilerWindow(ProfilerManager& manager);

		void Draw() override final;

		void DrawLoggerTab();

		void DrawPerformanceTab();
	};
}

#endif