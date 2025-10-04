#include <pch.h>
#include "Serializer/JSONSerializer.h"
#include "SceneSystem.h"
#include "Core/Core.h"

namespace SliceEngine
{
	void SceneSystem::LoadScene(std::filesystem::path const& filePath)
	{
		SLICE_LOG("Attempting to load scene from path: " + filePath.string());

		if (!std::filesystem::exists(filePath))
		{
			SLICE_LOG_ERROR("Filepath not found. Loading scene unsuccessful.");
			return;
		}

		mCurrentScene = filePath;

		SLICE_LOG("Loading scene...");

		JSONSerializer::DeserializeScene(filePath);

		SLICE_LOG("Scene loaded successfully.");
	}

	void SceneSystem::LoadScene(uint32_t const index)
	{
		current_scene = sceneMap[index];
		JSONSerializer::DeserializeScene(sceneMap[index]);
	}

	void SceneSystem::SaveScene(std::filesystem::path const& filePath)
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

	void SceneSystem::SaveTempScene()
	{
		// saves to temp scene
		// TODO: Create a global config that has a file path to asset folder
		JSONSerializer::SerializeScene("Assets/Temp/temp.scene");
		//upon exit of application need to delete the temp scene(?)
	}

	// For play then unplay, should call this one to reload scene as per last save instead of using current information
	void SceneSystem::ReloadScene()
	{
		// need function to clear everything on the scene from engine side

		// reloads the scene
		JSONSerializer::DeserializeScene(mCurrentScene);
	}

	void SceneSystem::Play()
	{
		//
		SaveTempScene();
	}

	void SceneSystem::Pause()
	{
		//
	}

	void SceneSystem::Stop()
	{
		//
		LoadScene(temp_scene);
	}

	std::filesystem::path SceneSystem::GetCurrentScenePath()
	{
		return mCurrentScene;
	}
}