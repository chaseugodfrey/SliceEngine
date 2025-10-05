#include <pch.h>
#include "Serializer/JSONSerializer.h"
#include "SceneSystem.h"

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

	std::filesystem::path SceneSystem::GetCurrentScenePath()
	{
		return mCurrentScene;
	}
}