#ifndef SCENE_SYSTEM_H
#define SCENE_SYSTEM_H

namespace SliceEngine
{
	class SceneSystem : BaseEngineSystem
	{
	public:
		void LoadScene(std::filesystem::path const& filePath);
		void SaveScene(std::filesystem::path const& filePath);
		void SaveTempScene();
		void ReloadScene();		
		
		void Play();
		void Pause();
		void Stop();

		// to be changed to use GUID maybe instead of string
		// Editor can use this to create the scene list
		std::vector<std::string> sceneMap;
	private:
		//can change to other identifier
		std::string current_scene;
		// store the temporary scene for editor
		const std::string temp_scene = "Assets/Temp/temp.scene";
	};
}


#endif