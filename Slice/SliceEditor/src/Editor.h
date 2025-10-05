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
#include "History/HistoryManager.h"
#include "WindowManager/WindowManager.h"
#include "SelectionSystem/SelectionSystem.h"
#include "AssetManager/AssetManager.h"

namespace SliceEditor
{
	class Editor
	{
		SliceEngine::Engine engine;
		Registry registry;

		HistoryManager history;
		AssetManager assetManager;

		void InitImGUI(GLFWwindow* window);
		void InitManagers();
		void InitEditorState();
		void InitWindowManager();
		void HandleDrop(const std::filesystem::path path);

		void CheckInputs();
		void Render();

	public:

		void Init();
		static void DropCallback(GLFWwindow* window, int count, const char** paths);
		static void MasterKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void MasterMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		void Run();
		void Exit();
	};
}

#endif