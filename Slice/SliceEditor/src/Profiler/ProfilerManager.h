/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        ProfilerManager.h

 author:	  Nic Lai

 email:       n.lai@digipen.edu

 brief:		  Declares the ProfilerManager class, which manages the data of the profiler window of the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef PROFILER_MANAGER_H
#define PROFILER_MANAGER_H

#include <memory>
#include "../Core/IBaseManager.h"
#include "../WindowManager/ICreateWindow.h"

namespace SliceEngine
{
	class FramerateManager;
}

namespace SliceEditor
{
	class Registry;

	class ProfilerManager : public IBaseManager, public ICreateWindow
	{
	private:
		struct DebugStats
		{
			float width;
			float timeTaken;
			float loadPercentage;
		};

		struct SystemHistory
		{
			std::deque<float> samples;
			float totalSum;
		};


		std::unordered_map<std::string, SystemHistory> mSystemMap;
		const size_t MAX_SAMPLES = 120;

	public:

		bool mAutoScroll = true;

		std::unordered_map<std::string, DebugStats> mDebugStats;

		float mCurrFPS;
		float mUntrackedFrameTime;
		float mUntrackedFrameTimePercentage;

		//SliceEngine::FramerateManager& framerateManager;

		ProfilerManager(Registry& reg) : IBaseManager(reg) {
			mUntrackedFrameTime = 0.0f;
		};
		~ProfilerManager() = default;

		void Init();
		void Update() override;

		ImVec4 LogLevelToImVec4(Logger::LogLevel level);

		std::unique_ptr<EditorWindow> CreateEditorWindow() override;

		void UpdateDebugStatistics();

		ImU32 GetSystemColor(const std::string& systemName);

		float LuminanceCalculation(ImU32 systemCol);
	};
}

#endif