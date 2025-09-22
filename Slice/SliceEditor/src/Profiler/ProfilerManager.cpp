#include <pch.h>
#include "ProfilerManager.h"
#include "ProfilerWindow.h"


namespace SliceEditor
{
	void ProfilerManager::Init()
	{
		SLICE_LOG("Initializing Profiler Data.");
	}

	std::unique_ptr<EditorWindow> ProfilerManager::CreateWindow()
	{
		SLICE_LOG("Creating Profiler Window.");
		auto window = std::make_unique<ProfilerWindow>(*this);

		return window;
	}
}