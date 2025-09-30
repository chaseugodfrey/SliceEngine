#ifndef SCENE_SYSTEM_H
#define SCENE_SYSTEM_H

#include <Serializer/JSONSerializer.h>

namespace SliceEngine
{
	class SceneSystem : BaseEngineSystem
	{
	public:
		void LoadScene(std::filesystem::path const& filePath);
		void SaveScene(std::filesystem::path const& filePath);

		void ReloadScene();		
		
		void Play();
		void Pause();
		void Stop();

	private:
		//can change to other identifier
		std::string current_scene;
	};
}


#endif