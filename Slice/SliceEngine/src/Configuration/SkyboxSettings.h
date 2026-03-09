#ifndef SKYBOX_SETTINGS_H
#define SKYBOX_SETTINGS_H

namespace SliceEngine
{
	struct SkyboxSettings : public ProjectSettings
	{
		SkyboxSettings(std::string name) : ProjectSettings(name) {};
		~SkyboxSettings() = default;

		void Init() override;
		void Exit() override;
		void LoadSettings(nlohmann::json) override;
		void SaveSettings() override;
		void ApplySettings() override;
	};
}

#endif