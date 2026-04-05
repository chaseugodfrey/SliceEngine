/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        Editor.h

 author:	  Chase Rodrigues
 co-author:   Nic Lai

 email:       rodrigues.i@digipen.edu

 brief:		  Declares the Editor class, which is the main class of the editor application.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef EDITOR_H
#define EDITOR_H

struct GLFWwindow;

#include "Engine.h"
#include "Core/Registry.h"
#include "AssetManager/AssetManager.h"
#include "AssetManager/AssetFileWatcher.h"
#include "EditorInputs.h"

namespace SliceEditor
{
	class Editor
	{
		SliceEngine::Engine engine;
		AssetManager assetManager;
		Registry registry;
		std::unique_ptr<EditorInputs> inputs;

		void InitImGUI(GLFWwindow* window);
		void InitManagers();
		void InitEditorState();
		void InitWindowManager();
		void HandleDrop(const std::filesystem::path path);

		void Update();
		void Render();
		void Save();

	public:

		Editor() : assetManager(), registry(assetManager)
		{
		}

		// todo : push this to event manager
		bool isNewSceneLoaded{};

		static void DropCallback(GLFWwindow* window, int count, const char** paths);
		static void MasterKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void MasterMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		void Init();
		void Run();
		void Exit();
	};
}

#endif