#ifndef PROJECT_SETTINGS_WINDOW_H
#define PROJECT_SETTINGS_WINDOW_H

#include "WindowManager/EditorWindow.h"

namespace SliceEditor
{
	class Registry;

	struct BaseSettingsDisplay
	{
	protected:
		Registry& mRegistry;

	public:
		std::string name;

		BaseSettingsDisplay(Registry& reg, std::string nm) : mRegistry(reg), name(nm) {};
		void DisplayHeader();
		virtual void DisplaySettings() = 0;
	};

	struct AudioSettingsDisplay : BaseSettingsDisplay
	{
		AudioSettingsDisplay(Registry& reg, std::string nm) : BaseSettingsDisplay(reg, nm) {};
		void DisplaySettings() override;
	};

	struct PhysicsSettingsDisplay : BaseSettingsDisplay
	{
		ImGuiTableColumnFlags column_flags = ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed;

		PhysicsSettingsDisplay(Registry& reg, std::string nm) : BaseSettingsDisplay(reg, nm) {};
		void DisplaySettings() override;
	};

	struct ProjectSettingsDisplay : BaseSettingsDisplay
	{
		ProjectSettingsDisplay(Registry& reg, std::string nm) : BaseSettingsDisplay(reg, nm) {};
		void DisplaySettings() override;
	};

	class ProjectSettingsWindow : public EditorWindow
	{
		enum class SettingsType : size_t
		{
			AUDIO,
			PHYSICS,
			PROJECT
		} mCurrentSettingsIndex;

		std::vector<std::unique_ptr<BaseSettingsDisplay>> mSettingsList{};


	public:
		
		ProjectSettingsWindow(Registry& reg) : EditorWindow(reg), mCurrentSettingsIndex(SettingsType::AUDIO) { };
		~ProjectSettingsWindow() = default;
		void Init() override;
		void Draw() override final;
	};
}

#endif