/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			SceneSystem.cpp
 author:		Hafiz
 co-author:		Lee Yong Yee
 email:			b.muhammadhafiz@digipen.edu
 brief:			Handles Scenes

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "Serializer/JSONSerializer.h"
#include "Core/Core.h"
#include "SceneSystem.h"

namespace SliceEngine
{
	void SceneSystem::Init()
	{
		//if (!LoadScene(mCurrentScene))
		//	LoadDefaultScene();

		//mCurrentState = mNextState = SceneState::DEFAULT;

		EventManager::GetInstance()->Subscribe<OnPlayEvent, &SceneSystem::OnPlay>(this);
	}

	void SceneSystem::LoadSceneIntoQueue(std::filesystem::path const filePath)
	{
		mSceneQueue.push(filePath);
	}

	void SceneSystem::LoadDefaultScene()
	{
		//Core::GetInstance()->mFactory.BuildSceneGraph();
		EventManager::GetInstance()->Publish<OnSceneLoadedEvent>(true);
	}

	bool SceneSystem::LoadSceneFromQueue()
	{
		auto asset_directory_path = std::filesystem::path("Assets");
		auto next_scene_filepath = mSceneQueue.front();
		mSceneQueue.pop();

		// Check if initial scene is empty, then load default scene
		if (next_scene_filepath.empty())
		{
			SLICE_LOG("No scene to load. Loading Default Scene.");
			LoadDefaultScene();
			return true;
		}

		auto next_scene_filename = std::filesystem::relative(next_scene_filepath, asset_directory_path).generic_string();

		SLICE_LOG("Attempting to load scene from path: " + next_scene_filepath.string());

		if (!std::filesystem::exists(next_scene_filepath))
		{
			SLICE_LOG("Failed to load scene from path: " + next_scene_filepath.string());
			return false;
		}

		UnloadCurrentScene();

		SLICE_LOG("Loading Scene: " + next_scene_filepath.string());

		auto map = JSONSerializer::DeserializeScene(next_scene_filepath);
		 
		mCurrentScene = next_scene_filepath;
		mCurrentSceneName = next_scene_filepath.stem().string();

		SLICE_LOG("Scene: " + next_scene_filepath.string() + " loaded successfully.");

		Core::GetInstance()->mFactory.BuildSceneGraph(map);
		Core::GetInstance()->mFactory.DebugPrint();

		OnSceneLoadedEvent event;
		event.isSceneLoaded = true;
		event.scenePath = mCurrentScene;
		EventManager::GetInstance()->Publish<OnSceneLoadedEvent>(event);

		if (next_scene_filepath.extension() == ".temp")
		{
			//std::filesystem::remove(next_scene_filepath);
			mCurrentScene.replace_extension(".scene");
		}

		return true;
	}

	void SceneSystem::LoadNavMeshFromMeta(std::filesystem::path metaFile)
	{
		std::ifstream meta(metaFile);

		nlohmann::json metaData;

		meta >> metaData;

		meta.close();

		std::filesystem::path navMeshFile(metaData["navMeshFile"].get<std::string>());

		if (std::filesystem::exists(navMeshFile))
		{
			//Do sth idk
		}
	}

	void SceneSystem::WriteTempFile()
	{
		std::filesystem::path CurrentScene = mCurrentScene;
		
		std::filesystem::path CurrentSceneTemp = CurrentScene;

		CurrentSceneTemp.replace_extension(".temp");

		JSONSerializer::SerializeScene(CurrentSceneTemp);
	}

	void SceneSystem::OnSceneSave(std::filesystem::path const filePath)
	{
		SLICE_LOG("Attempting to save scene from path: " + filePath.string());

		std::filesystem::path directory = filePath.parent_path();
		if (!std::filesystem::exists(directory))
		{
			try
			{
				std::filesystem::create_directories(directory);
				SLICE_LOG("Created directory: " + directory.string());
			}
			catch (const std::filesystem::filesystem_error& e)
			{
				SLICE_LOG_ERROR("Failed to create directory: " + directory.string() + ". Error: " + e.what());
				return; // Stop if we can't create the directory
			}
		}

		SLICE_LOG("Saving scene...");

		JSONSerializer::SerializeScene(filePath);

		SLICE_LOG("Scene saved successfully.");

	}

	void SceneSystem::SaveCurrentScene()
	{
		OnSceneSave(mCurrentScene);
	}

	void SceneSystem::SaveNextScene()
	{
		OnSceneSave(mNextScene);
	}

	void SceneSystem::UnloadCurrentScene()
	{
		if (mCurrentScene.empty())
		{
			SLICE_LOG("No scene is currently loaded. Skipping unload.");
			return;
		}

		SLICE_LOG("Unloading scene: " + mCurrentScene.string());

		Core::GetInstance()->mFactory.ClearGameObjects();
		Core::GetInstance()->mFactory.UpdateDestroyed();

		OnSceneStopEvent event;
		event.isSceneStopped = true;

		EventManager::GetInstance()->Publish<OnSceneStopEvent>(event);

		isSceneUnloaded = true;
	}

	// For play then unplay, should call this one to reload scene as per last save instead of using current information
	void SceneSystem::ReloadScene()
	{
		// need function to clear everything on the scene
		//Core::GetInstance()->mFactory.ClearGameObjects();
		//Core::GetInstance()->mFactory.UpdateDestroyed();

		std::filesystem::path CurrentScene = mCurrentScene;

		std::filesystem::path CurrentSceneTemp = CurrentScene;

		CurrentSceneTemp.replace_extension(".temp");

		if (std::filesystem::exists(CurrentSceneTemp))
		{
			mCurrentScene = CurrentSceneTemp;
		}

		LoadSceneIntoQueue(mCurrentScene);
		// reloads the scene
		/*JSONSerializer::DeserializeScene(mCurrentScene);

		Core::GetInstance()->mFactory.BuildSceneGraph(map);

		OnSceneLoadedEvent event;
		event.isSceneLoaded = true;

		EventManager::GetInstance()->Publish<OnSceneLoadedEvent>(event);*/
	}

	void SceneSystem::OnPlay(OnPlayEvent e)
	{
		Play();
	}

	void SceneSystem::Play()
	{
		mNextState = SceneState::PLAY_SCENE;
	}

	void SceneSystem::Pause()
	{
		//
		if (mCurrentState == SceneState::PLAY_SCENE)
		{
			mNextState = SceneState::PAUSE_SCENE;
		}
	}

	void SceneSystem::Stop()
	{
		if (mCurrentState == SceneState::PLAY_SCENE || mCurrentState == SceneState::PAUSE_SCENE)
		{
			
			mNextState = SceneState::STOP_SCENE;

		}
	}

	bool SceneSystem::CheckQueueEmpty()
	{
		return mSceneQueue.empty();
	}

	std::filesystem::path SceneSystem::GetCurrentScenePath()
	{
		return mCurrentScene;
	}

	std::string SceneSystem::GetCurrentSceneName()
	{
		if (mCurrentScene.empty())
			return "New Scene";

		return mCurrentSceneName;
	}
}