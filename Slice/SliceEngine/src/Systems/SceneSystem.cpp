/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			SceneSystem.cpp
 author:		Hafiz
 email:			b.muhammadhafiz@digipen.edu
 brief:			Handles Scenes

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "Serializer/JSONSerializer.h"
#include "SceneSystem.h"

namespace SliceEngine
{
	
	void SceneSystem::Init()
	{
		//Will do all the loading of the resources based on the scene file

	}
	void SceneSystem::LoadSceneIntoQueue(std::filesystem::path const filePath)
	{
		mSceneQueue.push(filePath);
		UnloadCurrentScene();
	}

	void SceneSystem::LoadScene(std::filesystem::path const filePath)
	{
		isSceneUnloaded = false;

		SLICE_LOG("Attempting to load scene from path: " + filePath.string());

		if (!std::filesystem::exists(filePath))
		{
			SLICE_LOG_ERROR("Filepath not found. Loading scene unsuccessful.");
			return;
		}

		mCurrentScene = filePath;

		SLICE_LOG("Loading scene...");

		auto map = JSONSerializer::DeserializeScene(filePath);

		SLICE_LOG("Scene loaded successfully.");

		Core::GetInstance()->mFactory.BuildSceneGraph(map);

		OnSceneLoadedEvent event;
		event.isSceneLoaded = true;

		EventManager::GetInstance()->Publish<OnSceneLoadedEvent>(event);
	}

	void SceneSystem::LoadNextScene()
	{
		auto scene_to_load = mSceneQueue.front();
		mSceneQueue.pop();
		LoadScene(scene_to_load);
	}

	void SceneSystem::SaveScene(std::filesystem::path const filePath)
	{
		SLICE_LOG("Attempting to save scene from path: " + filePath.string());

		if (!std::filesystem::exists(filePath))
		{
			SLICE_LOG_ERROR("Filepath not found. Saving scene unsuccessful.");
			return;
		}

		SLICE_LOG("Saving scene...");

		JSONSerializer::SerializeScene(filePath);

		SLICE_LOG("Scene saved successfully.");

	}

	void SceneSystem::SaveCurrentScene()
	{
		SaveScene(mCurrentScene);
	}

	void SceneSystem::UnloadCurrentScene()
	{
		SLICE_LOG("Unloading Scenes.");

		Core::GetInstance()->mFactory.ClearGameObjects();
	}

	// For play then unplay, should call this one to reload scene as per last save instead of using current information
	void SceneSystem::ReloadScene()
	{
		// need function to clear everything on the scene

		// reloads the scene
		JSONSerializer::DeserializeScene(mCurrentScene);
	}

	void SceneSystem::Play()
	{
		//
	}

	void SceneSystem::Pause()
	{
		//
	}

	void SceneSystem::Stop()
	{
		//
	}

	bool SceneSystem::CheckQueueEmpty()
	{
		return mSceneQueue.empty();
	}

	std::filesystem::path SceneSystem::GetCurrentScenePath()
	{
		return mCurrentScene;
	}
}