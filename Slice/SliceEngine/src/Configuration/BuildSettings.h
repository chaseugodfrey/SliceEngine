#ifndef BUILD_SETTINGS_H
#define BUILD_SETTINGS_H

namespace SliceEngine
{
	struct BuildSettings : public ProjectSettings
	{
		BuildSettings(std::string name) : ProjectSettings(name) {};
		~BuildSettings() = default;

		void Init() override;
		void Exit() override;
		void LoadSettings(nlohmann::json) override;
		void SaveSettings() override;
		void ApplySettings() override;
	};
}

#endif