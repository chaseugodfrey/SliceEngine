#ifndef SCENE_SYSTEM_H
#define SCENE_SYSTEM_H

namespace SliceEngine
{
	class SceneSystem : BaseEngineSystem
	{
	public:
		void Init();

		void LoadScene(std::filesystem::path const& filePath);
		void SaveScene(std::filesystem::path const& filePath);
		void SaveCurrentScene();

		void ReloadScene();		
		
		void Play();
		void Pause();
		void Stop();

		std::filesystem::path GetCurrentScenePath();

	private:
		//can change to other identifier
		std::filesystem::path mCurrentScene{};
	};
}


#endif