#ifndef PREFERENCE_WINDOW_H
#define PREFERENCE_WINDOW_H

#include "WindowManager/EditorWindow.h"

namespace SliceEditor
{
	class PreferenceWindow : public EditorWindow
	{
		Preferences* preferences;
		void SetThemeColor(ImVec4 col);

	public:
		PreferenceWindow(Registry& reg) : EditorWindow(reg) { preferences = nullptr; };
		~PreferenceWindow() = default;
		void Init() override;
		void Draw() override final;
	};
}

#endif