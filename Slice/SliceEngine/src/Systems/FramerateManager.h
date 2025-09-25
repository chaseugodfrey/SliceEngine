#ifndef FRAMERATE_MANAGER_H
#define FRAMERATE_MANAGER_H

namespace SliceEngine
{

	class FramerateManager
	{

	public:
		using Clock = std::chrono::high_resolution_clock;
		using TimePoint = std::chrono::time_point<Clock>;
		void Init();
		void StartFrame();
		void EndFrame();
		void StartSystem(const std::string &name);  
		void EndSystem(const std::string &name);
		float GetCurrFPS();

		std::unordered_map<std::string, std::pair<TimePoint,TimePoint>> systemStartEndTimes;
		std::unordered_map<std::string, float> systemDurations;
	private:
		TimePoint frameStartTime;
		TimePoint frameEndTime;
		float currFPS;
		bool firstFrameDone = false;

	};
}

#endif