#ifndef EDITOR_H
#define EDITOR_H



struct GLFWwindow;

#include "Engine.h"
#include "SceneView/SceneViewManager.h"
#include "ContentBrowser/ContentBrowserManager.h"
#include "History/HistoryManager.h"
#include "WindowManager/WindowManager.h"
#include "Hierachy/HierarchyManager.h"

namespace SliceEditor
{
	class Editor
	{
		SliceEngine::Engine engine;

		std::unique_ptr<SceneViewManager> sceneViewManager;
		std::unique_ptr<HierarchyManager> hierarchyManager;
		ContentBrowserManager contentBrowserManager;
		ProfilerManager profilerManager;
		HistoryManager history;
		WindowManager windowManager;

		//std::unique_ptr<EditorState> editorState;
		//std::unique_ptr<WindowManager> windowManager;

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