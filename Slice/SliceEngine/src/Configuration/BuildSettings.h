#ifndef BUILD_SETTINGS_H
#define BUILD_SETTINGS_H

namespace SliceEngine
{
	struct BuildSettings : public ProjectSettings
	{
		struct SceneEntry
		{
			std::string filename;
			Handle<SliceEngineTypes::Scene> handle;
		};

		std::vector<SceneEntry> mSceneList{};

		BuildSettings(std::string name) : ProjectSettings(name) {};
		~BuildSettings() = default;

		void Init() override;
		void Exit() override;
		void LoadSettings(nlohmann::json) override;
		void SaveSettings() override;
		void ApplySettings() override;

		Handle<SliceEngineTypes::Scene> GetSceneHandleByIndex(size_t index);
		Handle<SliceEngineTypes::Scene> GetSceneHandleByName(const std::string& name);
	};
}

#endif