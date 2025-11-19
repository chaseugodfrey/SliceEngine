#ifndef PROJECT_SETTINGS_WINDOW_H
#define PROJECT_SETTINGS_WINDOW_H

#include "WindowManager/EditorWindow.h"

namespace SliceEditor
{
	class ProjectSettingsWindow : public EditorWindow
	{
		enum class SettingsType : size_t
		{
			AUDIO,
			PHYSICS,
			PROJECT
		} mCurrentSettingsIndex;

		struct DSettings
		{
			std::string name;

			DSettings(std::string nm) : name(nm) {};
			void DisplayHeader();
			virtual void DisplaySettings() = 0;
		};

		struct DAudioSettings : DSettings
		{
			DAudioSettings(std::string nm) : DSettings(nm) {};
			void DisplaySettings() override;
		};

		struct DPhysicsSettings : DSettings
		{
			DPhysicsSettings(std::string nm) : DSettings(nm) {};
			void DisplaySettings() override;
		};

		struct DProjectSettings : DSettings
		{
			DProjectSettings(std::string nm) : DSettings(nm) {};
			void DisplaySettings() override;
		};

		std::vector<std::unique_ptr<DSettings>> mSettingsList{};

	public:
		
		ProjectSettingsWindow(Registry& reg) : EditorWindow(reg), mCurrentSettingsIndex(SettingsType::AUDIO) { };
		~ProjectSettingsWindow() = default;
		void Init() override;
		void Draw() override final;
	};
}

#endif