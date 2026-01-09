#ifndef PROJECT_SETTINGS_MANAGER_H
#define PROJECT_SETTINGS_MANAGER_H

#include "ProjectSettingsIncludes.h"

namespace SliceEngine
{
	class ProjectSettingsManager
	{
		std::vector<std::unique_ptr<ProjectSettings>> mSettingsList{};

	public:

		void Init();
		void Load();
		void Save();
		void Exit();

		template<typename Settings>
		Settings* GetSettings()
		{
			
			for (auto& setting : mSettingsList)
			{
				if (auto casted = dynamic_cast<Settings*>(setting.get()))
				{
					return casted;
				}
			}
			return nullptr;
		}
	};
}

#endif