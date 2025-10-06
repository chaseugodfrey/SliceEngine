/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			SceneSystem.h
 author:		Hafiz
 email:			b.muhammadhafiz@digipen.edu
 brief:			Handles Scenes

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
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