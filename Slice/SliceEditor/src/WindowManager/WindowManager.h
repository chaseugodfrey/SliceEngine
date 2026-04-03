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

#include "../Core/IBaseManager.h"
#include "WindowTypes.h"
#include "EditorWindow.h"

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
		void DrawPreferenceSettings();
		void DrawProjectSettings();
		void DrawSaveSceneAsPopup();
		void DrawNewScenePopup();

		void DrawSavePopupModal();

		void DrawUndoDisabledPopup();

		void QuitGameEvent(OnGameStopEvent e);

		bool projectSettingsPopupOpen{ false };
		bool saveSceneAsPopup{ false };
		bool saveScenePopupOpen{ false };
		bool saveScenePopupClose{ false };
		bool newScenePopupOpen{ false };
		bool preferenceSettingsPopupOpen{ false };
		bool isPlaying{ false }; // --TODO-- Change Reading from Somewhere else(?)
		bool isPaused{ false };
		bool undoDisabledEvent{ false };

	public:
		
		WindowManager(Registry& reg) : IBaseManager(reg) {};
		~WindowManager() = default;

		//void Init(EditorState& editorState);

		template <typename WindowType>
		inline bool CheckIfWindowExists()
		{
			for (auto& window : list) {
				if (dynamic_cast<WindowType*>(window.get())) {
					return true;
				}
			}

			return false;
		}

		template <typename WindowType>
		inline std::optional<WindowType*> GetWindow()
		{
			for (auto& window : list) {
				if (auto result = dynamic_cast<WindowType*>(window.get())) {
					return result;
				}
			}
			return std::nullopt;
		}

		template <typename WindowType>
		inline void AddWindow()
		{
			if (CheckIfWindowExists<WindowType>())
			{
				SLICE_LOG("Only one instance of this window can exist.");
				return;
			}

			static_assert(std::is_base_of_v<EditorWindow, WindowType>, "WindowType must derive from EditorWindow");

			auto window = std::make_unique<WindowType>(registry);
			window->Init();
			list.push_back(std::move(window));
		}

		template <typename WindowType>
		inline void AddWindow(const char* name)
		{
			auto it = windowFactoryMap.find(std::string(name));
			if (it != windowFactoryMap.end())
			{
				if (CheckIfWindowExists<WindowType>())
				{
					SLICE_LOG("Only one instance of this window can exist.");
					return;
				}

				auto window = it->second->CreateEditorWindow();
				list.push_back(std::move(window));
			}
			else
			{
				assert(true);
			}
		}

		void Init();
		void Update() override;
		void RegisterInterface(const std::string& name, ICreateWindow* interfaceInstance);
		void Render();

		void OpenSaveScenePopup();
		void CloseSaveScenePopup();

		void MenuToggleBit(const char* label, unsigned char& mask, unsigned char bit);
		void SetUndoDisabled();
	};
}



#endif