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
	private:
		struct DebugStats
		{
			float width;
			float timeTaken;
			float loadPercentage;
		};


		//std::queue<std::string> textStack;

	public:

		bool mAutoScroll = true;

		std::unordered_map<std::string, DebugStats> mDebugStats;

		//SliceEngine::FramerateManager& framerateManager;

		ProfilerManager() = default;
		~ProfilerManager() = default;

		void Init();

		ImVec4 LogLevelToImVec4(Logger::LogLevel level);

		std::unique_ptr<EditorWindow> CreateWindow() override;

		void UpdateDebugStatistics();

		ImU32 GetSystemColor(const std::string& systemName);

		float LuminanceCalculation(ImU32 systemCol);
	};
}

#endif