#ifndef SCENE_SYSTEM_H
#define SCENE_SYSTEM_H

namespace SliceEngine
{
	class SceneSystem : BaseEngineSystem
	{
	public:
		void Init();

		void LoadSceneIntoQueue(std::filesystem::path const filePath);
		void LoadScene(std::filesystem::path const filePath);
		void LoadScene(uint32_t const index);
		void LoadNextScene();
		void SaveScene(std::filesystem::path const filePath);
		void SaveCurrentScene();
		void UnloadCurrentScene();
		void ReloadScene();		
		
		void Play();
		void Pause();
		void Stop();
		bool IsSceneUnloaded();
		bool CheckQueueEmpty();

		std::filesystem::path GetCurrentScenePath();
		bool isSceneUnloaded{};

	private:
		//can change to other identifier
		std::queue<std::filesystem::path> mSceneQueue;
		std::filesystem::path mCurrentScene{};
	};
}


#endif