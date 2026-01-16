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
		if (!LoadScene(mCurrentScene))
			LoadDefaultScene();

		mCurrentState = mNextState = SceneState::DEFAULT;

		EventManager::GetInstance()->Subscribe<OnPlayEvent, &SceneSystem::OnPlay>(this);
	}

	void SceneSystem::LoadSceneIntoQueue(std::filesystem::path const filePath)
	{
		mSceneQueue.push(filePath);
		mNextScene = filePath;
		UnloadCurrentScene();
		
	}

	bool SceneSystem::LoadScene(GUID const guid)
	{
		auto resourceManager = Core::GetInstance()->GetResourceManager();
		auto scene = resourceManager->get<SliceEngineTypes::Scene>(guid);

		if (!scene.IsValid())
			return false;
		
		return LoadScene(scene->GetFilePath());
	}

	bool SceneSystem::LoadScene(SliceEngineTypes::Scene const* scene)
	{
		return LoadScene(scene->GetFilePath());
	}

	void SceneSystem::LoadDefaultScene()
	{
		//Core::GetInstance()->mFactory.BuildSceneGraph();
		EventManager::GetInstance()->Publish<OnSceneLoadedEvent>(true);
	}

	bool SceneSystem::LoadScene(std::filesystem::path const filePath)
	{
		SLICE_LOG("Attempting to load scene from path: " + filePath.string());

		std::filesystem::path mAssetDirectory = std::filesystem::path("Assets");

		mCurrentSceneName = std::filesystem::relative(filePath, mAssetDirectory).generic_string();

		mCurrentScene = filePath;

		// auto filePathGUID = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Scene>(filePath.stem().string()).get();
		// if (filePath.extension() == ".temp")
		// {
		// 	SLICE_LOG("Loading scene...");

		// 	auto map = JSONSerializer::DeserializeScene(filePath);

		// 	SLICE_LOG("Scene loaded successfully.");

		// 	Core::GetInstance()->mFactory.BuildSceneGraph(map);
		// 	Core::GetInstance()->mFactory.DebugPrint();
		// 	OnSceneLoadedEvent event;
		// 	event.isSceneLoaded = true;

		// 	EventManager::GetInstance()->Publish<OnSceneLoadedEvent>(event);

		// 	return;
		// }

		auto resourceMgr = Core::GetInstance()->GetResourceManager();
		auto filePathGUID = resourceMgr->get<SliceEngineTypes::Scene>(mCurrentSceneName).get();
		if (filePathGUID)
		{
			
			std::filesystem::path filePathToLoad = filePathGUID->GetFilePath();


			SLICE_LOG("Loading scene...");

			auto map = JSONSerializer::DeserializeScene(filePathToLoad);

			if (mCurrentScene.extension() == ".temp")
			{
				// this only happens in editor mode
				// technically temp file shouldn't even be creating a resource i think
				// so it doesn't have to be cleaned up, 
				if (std::filesystem::exists(mCurrentScene))
				{
					GUID fileGUID = resourceMgr->mFileNameToGUID[mCurrentSceneName];
					resourceMgr->ReleaseResource(fileGUID);

					// delete all 3 files
					std::filesystem::remove(mCurrentScene);
					std::filesystem::remove(filePathToLoad);
					std::filesystem::path newPath = mCurrentScene;
					mCurrentScene += ".meta";
					std::filesystem::remove(mCurrentScene);

					// change back to the .scene one
					newPath.replace_extension(".scene");
					mCurrentScene = newPath;
					//filePathToLoad = mCurrentScene;
				}
			}


			SLICE_LOG("Scene loaded successfully.");

			Core::GetInstance()->mFactory.BuildSceneGraph(map);
			Core::GetInstance()->mFactory.DebugPrint();

			EventManager::GetInstance()->Publish<OnSceneLoadedEvent>(true);

			return true;
		}

		return false;
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

	void SceneSystem::LoadNextScene()
	{
		/*auto scene_to_load = mSceneQueue.front();
		mSceneQueue.pop();
		LoadScene(scene_to_load);*/
		if (mNextScene == mSceneQueue.front())
		{
			
			LoadScene(mNextScene);
			mSceneQueue.pop();
			mNextScene = "";
		}
	}

	void SceneSystem::WriteTempFile()
	{
		std::filesystem::path CurrentScene = mCurrentScene;
		
		std::filesystem::path CurrentSceneTemp = CurrentScene;

		CurrentSceneTemp.replace_extension(".temp");

		JSONSerializer::SerializeScene(CurrentSceneTemp);
	}

	void SceneSystem::SetCurrentScenePath(std::filesystem::path const& filePath)
	{
		mCurrentScene = filePath;
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
		SLICE_LOG("Unloading Scenes.");

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

		return mCurrentScene.stem().string();
	}
}