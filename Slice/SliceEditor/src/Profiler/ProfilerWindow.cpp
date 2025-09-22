#include <pch.h>
#include "ProfilerWindow.h"

namespace SliceEditor
{
	ProfilerWindow::ProfilerWindow(ProfilerManager& man) : manager(man)
	{
	}

	void ProfilerWindow::Draw()
	{
		ImGui::Begin("Profiler");

		ImGui::End();
	}
}