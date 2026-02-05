#ifndef PREFERENCE_WINDOW_H
#define PREFERENCE_WINDOW_H

#include "WindowManager/EditorWindow.h"

namespace SliceEditor
{
	struct BasePreferenceDisplay
	{
	protected:
		Registry& mRegistry;

	public:
		std::string const name;
		BasePreferenceDisplay(Registry& reg, std::string nm) : mRegistry(reg), name(nm) {};
		void DisplayHeader();
		virtual void DisplayPreferences(Preferences&) = 0;
	};

	struct ThemePreferenceDisplay : BasePreferenceDisplay
	{
		ThemePreferenceDisplay(Registry& reg, std::string nm) : BasePreferenceDisplay(reg, nm) {};
		void DisplayPreferences(Preferences&) override;
	};

	struct ScenePreferenceDisplay : BasePreferenceDisplay
	{
		ScenePreferenceDisplay(Registry& reg, std::string nm) : BasePreferenceDisplay(reg, nm) {};
		void DisplayPreferences(Preferences&) override;
	};

	class PreferenceWindow : public EditorWindow
	{
		enum class PreferenceType : size_t
		{
			THEME,
			SCENE
		} mCurrentPreferenceIndex{};

		std::vector<std::unique_ptr<BasePreferenceDisplay>> mPreferenceList{};

	public:
		PreferenceWindow(Registry& reg) : EditorWindow(reg) { };
		~PreferenceWindow() = default;
		void Init() override;
		void Draw() override final;
	};
}

#endif