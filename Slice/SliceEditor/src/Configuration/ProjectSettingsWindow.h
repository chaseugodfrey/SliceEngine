#ifndef PROJECT_SETTINGS_WINDOW_H
#define PROJECT_SETTINGS_WINDOW_H

#include "WindowManager/EditorWindow.h"
#include <Configuration/ProjectSettingsIncludes.h>

namespace SliceEditor
{
	class Registry;

	struct BaseSettingsDisplay
	{
	protected:
		Registry& mRegistry;

	public:
		std::string const name;
		SliceEngine::ProjectSettings& mSettings;

		BaseSettingsDisplay(Registry& reg, SliceEngine::ProjectSettings& settings, std::string nm) : mRegistry(reg), mSettings(settings), name(nm) {};
		void DisplayHeader();
		virtual void DisplaySettings(ImVec2 size) = 0;
	};

	struct AudioSettingsDisplay : BaseSettingsDisplay
	{
		AudioSettingsDisplay(Registry& reg, SliceEngine::ProjectSettings& stg, std::string nm) : BaseSettingsDisplay(reg, stg, nm) {};
		void DisplaySettings(ImVec2 size) override;
	};

	struct PhysicsSettingsDisplay : BaseSettingsDisplay
	{
		ImGuiTableColumnFlags column_flags = ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_WidthFixed;

		PhysicsSettingsDisplay(Registry& reg, SliceEngine::ProjectSettings& stg, std::string nm) : BaseSettingsDisplay(reg, stg, nm) {};
		void DisplaySettings(ImVec2 size) override;
	};

	struct BuildSettingsDisplay : BaseSettingsDisplay
	{
	private:
		bool AddSceneToList(std::vector<SliceEngine::Handle<SliceEngine::SliceEngineTypes::Scene>>& list, SliceEngine::GUID guid);
	public:
		BuildSettingsDisplay(Registry& reg, SliceEngine::ProjectSettings& stg, std::string nm) : BaseSettingsDisplay(reg, stg, nm) {};
		void DisplaySettings(ImVec2 size) override;
	};

	class ProjectSettingsWindow : public EditorWindow
	{
		enum class SettingsType : size_t
		{
			AUDIO,
			PHYSICS,
			BUILD
		} mCurrentSettingsIndex{};

		std::vector<std::unique_ptr<BaseSettingsDisplay>> mSettingsList{};

	public:
		
		ProjectSettingsWindow(Registry& reg) : EditorWindow(reg) { };
		~ProjectSettingsWindow() = default;
		void Init() override;
		void Draw() override final;
	};
}

#endif