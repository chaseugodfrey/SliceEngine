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

		mCurrentScene = filePath.string();

		SLICE_LOG("Loading scene...");

		JSONSerializer::DeserializeScene(filePath);

		SLICE_LOG("Scene loaded successfully.");
	}

	void SceneSystem::SaveScene(std::filesystem::path const& filePath)
	{
		SLICE_LOG("Attempting to save scene from path: " + filePath.string());

		mCurrentScene = filePath.string();

		SLICE_LOG("Saving scene...");

		JSONSerializer::SerializeScene(filePath);

		SLICE_LOG("Scene saved successfully.");

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
}