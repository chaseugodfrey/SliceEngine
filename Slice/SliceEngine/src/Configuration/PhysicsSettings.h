#ifndef PHYSICS_SETTINGS_H
#define PHYSICS_SETTINGS_H

namespace SliceEngine
{
	struct PhysicsSettings : public ProjectSettings
	{
		PhysicsSettings(std::string name) : ProjectSettings(name) {};
		void LoadSettings() override;
		void SaveSettings() override;
	};
}

#endif