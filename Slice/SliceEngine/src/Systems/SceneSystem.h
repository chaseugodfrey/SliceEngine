#ifndef SCENE_SYSTEM_H
#define SCENE_SYSTEM_H

namespace SliceEngine
{
	// todo: transfer this to a scene asset/read this from a scene asset
	struct Scene
	{
		std::wstring name;
		std::wstring path;
	};

	class SceneSystem : BaseEngineSystem
	{
	public:
		void Init();

		void LoadScene(std::filesystem::path const& filePath);
		void SaveScene(std::filesystem::path const& filePath);

		void ReloadScene();		
		
		void Play();
		void Pause();
		void Stop();

	private:
		//can change to other identifier
		std::string mCurrentScene;
		std::map<int, Scene> mScenes;
	};
}


#endif