#ifndef PREFERENCE_MANAGER_H
#define PREFERENCE_MANAGER_H

#include "Core/IBaseManager.h"

namespace SliceEditor
{
	class Registry;

	class PreferenceManager : public IBaseManager
	{
		std::unique_ptr<Preferences> mPreferences;

	public:
		PreferenceManager(Registry& reg) : IBaseManager(reg) {};
		~PreferenceManager() = default;

		void Init() override;
		void Update() override;
		void CreateDefaultPreferenceFile();
		void LoadPreferences();
		void SetPreferences();
		void SavePreferences();

		Preferences& GetPreferences();
	};
}

#endif