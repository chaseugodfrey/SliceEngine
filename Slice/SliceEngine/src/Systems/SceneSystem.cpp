#include <pch.h>
#include "Serializer/JSONSerializer.h"
#include "SceneSystem.h"
#include "Core/Core.h"

namespace SliceEngine
{
	void SceneSystem::LoadScene(std::filesystem::path const& filePath)
	{
		// need function to clear everything on the scene
		

		current_scene = filePath.string();
		JSONSerializer::DeserializeScene(filePath);
	}

	void SceneSystem::SaveScene(std::filesystem::path const& filePath)
	{
		current_scene = filePath.string();
		JSONSerializer::SerializeScene(filePath);
	}

	void SceneSystem::SaveTempScene()
	{
		// saves to temp scene
		// TODO: Create a global config that has a file path to asset folder
		JSONSerializer::SerializeScene("Assets/Temp/temp.scene");
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
}