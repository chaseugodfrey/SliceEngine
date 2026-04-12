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
#include "Configuration/ProjectSettingsManager.h"
#include "Configuration/BuildSettings.h"
#include "../Graphics/CanvasSystem.h"
#include "../Graphics/UI_Interactible.h"
#include "../Scripting/ScriptSystem.h"
#include "../Animator/AnimatorSystem.h"

namespace SliceEngine
{
	void SceneSystem::Init()
	{
		mCurrentState = mNextState = SceneState::DEFAULT;
		EventManager::GetInstance()->Subscribe<OnPlayEvent, &SceneSystem::OnPlay>(this);
	}

	void SceneSystem::LoadSceneIntoQueue(std::filesystem::path const filePath)
	{
		mSceneQueue.push(filePath);
	}

	void SceneSystem::LoadDefaultScene()
	{
		//EventManager::GetInstance()->Publish<OnSceneLoadedEvent>(true);
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


		if (mCurrentState == SceneState::PLAY_SCENE || mCurrentState == SceneState::RELOAD_SCENE)
		{
			// publish event to scene change
			OnSceneChangeEvent ChangeEvent;
			EventManager::GetInstance()->Publish<OnSceneChangeEvent>(ChangeEvent);

			//if (mCurrentState == SceneState::PLAY_SCENE && mNextState == SceneState::PLAY_SCENE)
			//{
			//	mCurrentState = SceneState::DEFAULT;
			//}


		}

		gScriptSystem->isChangingScene = true;

		UnloadCurrentScene();

		SLICE_LOG("Loading Scene: " + next_scene_filepath.string());

		auto map = JSONSerializer::DeserializeScene(next_scene_filepath);

		//Call DeserializeSceneNavMesh function, will return a guid
		auto navMeshBinGUID = JSONSerializer::DeserializeNavMeshBinGUID(next_scene_filepath);
		 

		mCurrentScene = next_scene_filepath;
		mCurrentSceneName = next_scene_filepath.stem().string();

		SLICE_LOG("Scene: " + next_scene_filepath.string() + " loaded successfully.");

		Core::GetInstance()->mFactory.BuildSceneGraph(map);
		Core::GetInstance()->mFactory.DebugPrint();

		/*std::filesystem::path metaPath = next_scene_filepath;

		std::string navMesh = "";
		metaPath += ".meta";
		

		navMesh = LoadNavMeshFromMeta(metaPath);*/

		std::string navMeshBinString = "Resources/";
		navMeshBinString += navMeshBinGUID.toString();

		OnSceneLoadedEvent event;
		event.isSceneLoaded = true;
		event.navMeshBinPath = navMeshBinString;
		EventManager::GetInstance()->Publish<OnSceneLoadedEvent>(event);



		if (next_scene_filepath.extension() == ".temp")
		{
			//std::filesystem::remove(next_scene_filepath);
			mCurrentScene.replace_extension(".scene");
		}

		if (mCurrentState == SceneState::RELOAD_SCENE && mNextState == SceneState::RELOAD_SCENE)
		{
			mNextState = mCurrentState = SceneState::DEFAULT;

		}

		/* NOTE FOR WRITING SCRIPTS:
		If a script that is ran from deserializing scene
		creates an object on create
		and that object references something in the scene
		it might crash cause on awake and oncreate might not have ran yet
		*/
		gScriptSystem->isChangingScene = false;

		auto& sCanvas = SliceEngine::Core::GetInstance()->GetSystem<CanvasSystem>();
		auto& sButton = SliceEngine::Core::GetInstance()->GetSystem<ButtonSystem>();
		auto& sAnimator = SliceEngine::Core::GetInstance()->GetSystem<AnimatorSystem>();

		sCanvas.UpdateHierachy(true);	//force all ui components to update once regardless of inactive
		gScriptSystem->UpdateScripts();
		//sCanvas.UpdateHierachy(true);	//force all ui components to update once regardless of inactive
		sButton.InitSystem();
		sAnimator.InitSystem();

		return true;
	}

	std::string SceneSystem::LoadNavMeshFromMeta(std::filesystem::path metaFile)
	{
		if (!std::filesystem::exists(metaFile))
			return "";

		std::ifstream meta(metaFile);
		if (!meta.is_open())
			return "";

		nlohmann::json metaData;

		try {
			meta >> metaData;
		}
		catch (...) {
			return "";
		}
		meta.close();

		std::string navMeshPath = "";

		if (metaData.contains("navMeshBinFile"))
		{
			navMeshPath = metaData["navMeshBinFile"].get<std::string>();
			
		}

		return navMeshPath;
	}

	void SceneSystem::LoadSceneByIndex(size_t index)
	{
		auto handle = Core::GetInstance()->GetProjectSettingsManager()->GetSettings<BuildSettings>()->GetSceneHandleByIndex(index);
		if (!handle.IsValid())
			return;

		LoadSceneIntoQueue(handle->GetFilePath());
	}

	void SceneSystem::LoadSceneByName(std::string const& name)
	{
		auto handle = Core::GetInstance()->GetProjectSettingsManager()->GetSettings<BuildSettings>()->GetSceneHandleByName(name);
		if (!handle.IsValid())
			return;

		LoadSceneIntoQueue(handle->GetFilePath());
	}

	void SceneSystem::WriteTempFile()
	{
		if (!mCanWriteTempFiles)
		{
			return;
		}

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

	void SceneSystem::SaveScene(std::filesystem::path const filePath)
	{
		OnSceneSave(filePath);
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

	float SceneSystem::GetTimeScale() const
	{
		return timeScale;
	}

	void SceneSystem::SetTimeScale(float newTimeScale)
	{
		timeScale = newTimeScale;
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