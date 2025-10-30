/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        WindowManager.h

 author:	  Chase Rodgrigues

 email:       rodrigues.i@digipen.edu

 brief:		  Declares the WindowManager which is responsible for creating and managing all editor windows.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "WindowTypes.h"
#include "EditorWindow.h"
#include "../Core/IBaseManager.h"

namespace SliceEditor
{
	class WindowManager : public IBaseManager
	{
		std::vector<std::unique_ptr<EditorWindow>> list;
		std::unordered_map<std::string, ICreateWindow*> windowFactoryMap;

		//std::shared_ptr<EditorState> editorState;
		void DrawMainMenu();
		void DrawDockspace();
		void DrawPlayState();
		void DrawPreferenceWindow();
		void DrawProjectSettings();
		void DrawSaveSceneAsPopup();
		void DrawNewScenePopup();

		bool projectSettingsPopupOpen{ false };
		bool saveSceneAsPopupOpen{ false };
		bool newScenePopupOpen{ false };

	public:
		
		WindowManager(Registry& reg) : IBaseManager(reg) {};
		~WindowManager() = default;

		//void Init(EditorState& editorState);
		void AddWindow(const std::string& name);
		
		template <typename WindowType>
		void AddWindow()
		{
			static_assert(std::is_base_of_v<EditorWindow, WindowType>, "WindowType must derive from EditorWindow");
			auto window = std::make_unique<WindowType>(registry);
			window->Init();
			list.push_back(std::move(window));
		}

		void Init();
		void RegisterInterface(const std::string& name, ICreateWindow* interfaceInstance);
		void Render();

	};
}



#endif