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

		void LoadDefaultScene();
		void LoadSceneIntoQueue(std::filesystem::path const filePath);
		bool LoadSceneFromQueue();
		std::string LoadNavMeshFromMeta(std::filesystem::path navMeshFile);
		
		void SetTempFileSaving(bool enabled) { mCanWriteTempFiles = enabled; }
		
		void LoadSceneByIndex(size_t index);
		void LoadSceneByName(std::string const& name);

		void WriteTempFile();
		
		void OnSceneSave(std::filesystem::path const filePath);
		void SaveScene(std::filesystem::path const filePath);
		void SaveCurrentScene();
		void SaveNextScene();
		void UnloadCurrentScene();
		void ReloadScene();		

		void OnPlay(OnPlayEvent e);
		void Play();
		void Pause();
		void Stop();
		bool IsSceneUnloaded();
		bool CheckQueueEmpty();

		float GetTimeScale() const;
		void SetTimeScale(float newTimeScale);

		std::filesystem::path GetCurrentScenePath();
		std::string GetCurrentSceneName();
		bool isSceneUnloaded{};
		bool mCanWriteTempFiles = true;

	private:
		//can change to other identifier

		std::queue<std::filesystem::path> mSceneQueue;
		std::filesystem::path mCurrentScene{};
		std::filesystem::path mNextScene{};
		GUID mCurrentSceneGUID;
		
		std::string mCurrentSceneName{};

		float timeScale{ 1.0f };

	};
}


#endif