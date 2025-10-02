#include <pch.h>
#include "Serializer/JSONSerializer.h"
#include "SceneSystem.h"

namespace SliceEngine
{
	void SceneSystem::LoadScene(std::filesystem::path const& filePath)
	{
		current_scene = filePath.string();
		JSONSerializer::DeserializeScene(filePath);
	}

	void SceneSystem::SaveScene(std::filesystem::path const& filePath)
	{
		current_scene = filePath.string();
		JSONSerializer::SerializeScene(filePath);
	}

	// For play then unplay, should call this one to reload scene as per last save instead of using current information
	void SceneSystem::ReloadScene()
	{
		// need function to clear everything on the scene

		// reloads the scene
		JSONSerializer::DeserializeScene(current_scene);
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