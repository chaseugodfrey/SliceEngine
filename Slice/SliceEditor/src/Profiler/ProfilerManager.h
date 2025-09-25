#ifndef PROFILER_MANAGER_H
#define PROFILER_MANAGER_H

#include <memory>
#include "../WindowManager/ICreateWindow.h"
namespace SliceEngine
{
	class FramerateManager;
}


namespace SliceEditor
{

	class ProfilerManager : public ICreateWindow
	{

		//std::queue<std::string> textStack;

	public:

		bool autoScroll = true;

		//SliceEngine::FramerateManager& framerateManager;

		ProfilerManager() = default;
		~ProfilerManager() = default;

		void Init();

		ImVec4 LogLevelToImVec4(Logger::LogLevel level);

		std::unique_ptr<EditorWindow> CreateWindow() override;
	};
}

#endif