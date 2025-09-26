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
		void CapFPS(int maxFPS);
		void CalculateSystemPercentages();
		float GetCurrFPS();
		const std::unordered_map<std::string, std::pair<TimePoint, TimePoint>> GetSysStartEndTimes();
		const std::unordered_map<std::string, float> GetSysDurations();
		const std::unordered_map<std::string, float> GetSystemPercentages() const;
		const float GetFrameTime();

	private:
		std::unordered_map<std::string, std::pair<TimePoint,TimePoint>> systemStartEndTimes;
		std::unordered_map<std::string, float> systemDurations;
		std::unordered_map<std::string, float> mSystemPercentages;
		TimePoint frameStartTime;
		TimePoint frameEndTime;
		float currFPS;
		float mTotalFrameTime;
		bool firstFrameDone = false;

	};
}

#endif