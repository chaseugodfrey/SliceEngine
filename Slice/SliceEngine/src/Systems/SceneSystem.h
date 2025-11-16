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
	struct SceneData
	{
		std::wstring name;
		std::filesystem::path path;

	};

	enum SceneState
	{
		DEFAULT,
		//Play scene
		PLAY_SCENE,
		//Reload scene
		RELOAD_SCENE,
		//Pause scene
		PAUSE_SCENE,
		//
		STOP_SCENE
	};

	class SceneSystem : BaseEngineSystem
	{
	public:
		
		

		SceneState mCurrentState;
		SceneState mNextState;
		

		void Init();

		void LoadSceneIntoQueue(std::filesystem::path const filePath);
		void LoadScene(std::filesystem::path const filePath);
		void LoadScene(uint32_t const index);
		void LoadNextScene();
		void WriteTempFile();
		void SetCurrentScenePath(std::filesystem::path const& filePath);
		void SetDefaultScenePath(std::filesystem::path const& filePath);
		
		void SaveScene(std::filesystem::path const filePath);
		void SaveCurrentScene();
		void SaveNextScene();
		void UnloadCurrentScene();
		void ReloadScene();		

		void Play();
		void Pause();
		void Stop();
		bool IsSceneUnloaded();
		bool CheckQueueEmpty();

		std::filesystem::path GetCurrentScenePath();
		std::filesystem::path GetDefaultScenePath();
		std::string GetCurrentSceneName();
		bool isSceneUnloaded{};

	private:
		//can change to other identifier

		std::queue<std::filesystem::path> mSceneQueue;
		std::filesystem::path mCurrentScene{};	
		std::filesystem::path mNextScene{};
		std::filesystem::path mDefaultScene{};
		
	};
}


#endif