#ifndef EDITOR_H
#define EDITOR_H

struct GLFWwindow;

#include "Engine.h"
#include "Core/Registry.h"
#include "History/HistoryManager.h"
#include "WindowManager/WindowManager.h"
#include "SelectionSystem/SelectionSystem.h"

namespace SliceEditor
{
	class Editor
	{
		SliceEngine::Engine engine;
		Registry registry;

		ProfilerManager profilerManager;
		HistoryManager history;
		WindowManager windowManager;

		void InitImGUI(GLFWwindow* window);
		void InitManagers();
		void InitEditorState();
		void InitWindowManager();
		void HandleDrop(const std::filesystem::path path);

		void Render();

	public:

		void Init();
		static void DropCallback(GLFWwindow* window, int count, const char** paths);
		void Run();
		void Exit();
	};
}

#endif