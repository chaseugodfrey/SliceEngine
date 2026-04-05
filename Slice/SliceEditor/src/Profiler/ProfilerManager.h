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



		const size_t MAX_SAMPLES = 120;
		std::map<std::string, SystemHistory> mSystemMap;

	public:

		bool mAutoScroll = true;
		bool mClearStatistics = false;

		std::map<std::string, DebugStats> mDebugStats;

		float mCurrFPS;
		float mDeltaTime;
		float mTotalFrameTime;
		SystemHistory mAverageTotalFrameTime;
		float mUntrackedFrameTime;
		float mUntrackedFrameTimePercentage;

		//SliceEngine::FramerateManager& framerateManager;

		ProfilerManager(Registry& reg) : IBaseManager(reg) , mCurrFPS(0.0f), mDeltaTime(0.0f), mTotalFrameTime(0.0f), 
			mUntrackedFrameTime(0.0f), mUntrackedFrameTimePercentage(0.0f), mClearStatistics(false)
		{
		};
		~ProfilerManager() = default;

		void Init();
		void Update() override;

		ImVec4 LogLevelToImVec4(Logger::LogLevel level);

		std::unique_ptr<EditorWindow> CreateEditorWindow() override;

		void UpdateDebugStatistics();

		void ClearDebugStatistics();

		ImU32 GetSystemColor(const std::string& systemName);

		float LuminanceCalculation(ImU32 systemCol);
	};
}

#endif