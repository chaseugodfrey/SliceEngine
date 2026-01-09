#ifndef PHYSICS_SETTINGS_H
#define PHYSICS_SETTINGS_H

namespace SliceEngine
{
	struct PhysicsSettings : public ProjectSettings
	{
		PhysicsSettings(std::string name) : ProjectSettings(name) {};
		~PhysicsSettings() = default;

		void Init() override;
		void Exit() override;
		void LoadSettings(nlohmann::json) override;
		void SaveSettings() override;
		void ApplySettings() override;
	};
}

#endif