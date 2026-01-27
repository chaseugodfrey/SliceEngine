#ifndef PREFERENCE_MANAGER_H
#define PREFERENCE_MANAGER_H

#include "Core/IBaseManager.h"
namespace SliceEditor
{
	class Registry;

	class PreferenceManager : public IBaseManager
	{
		constexpr static unsigned int CURRENT_VERSION = 1;

		std::unique_ptr<Preferences> mPreferences;

	public:
		PreferenceManager(Registry& reg) : IBaseManager(reg) {};
		~PreferenceManager() = default;

		void Init() override;
		void Update() override;
		void UpdateVersion(nlohmann::json& preferences, unsigned int);
		void LoadPreferences();
		void SetPreferences();
		void SavePreferences();
		void UpdateLastSceneLoaded(OnSceneLoadedEvent e);

		Preferences& GetPreferences();
	};
}

#endif