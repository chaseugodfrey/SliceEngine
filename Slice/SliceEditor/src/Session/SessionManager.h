#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include "../Core/IBaseManager.h"

namespace SliceEditor
{
	class Registry;
	class SelectedNode;

	class SessionManager : public IBaseManager
	{
		std::unique_ptr<Preferences> mPreferences;

		std::unordered_map<entt::entity, std::unique_ptr<EntityNode>> mEntityNodes;

	public:
		SessionManager(Registry& reg);
		~SessionManager();

		void Init() override;
		void Update() override;

		void OpenPreferences();
		void SetPreferences();
		void CreateDefaultPreferenceFile();
		void SavePreferences();
		Preferences& GetPreferences();

		void CreateEntityNodes();
		std::unordered_map<entt::entity, std::unique_ptr<EntityNode>>& GetEntityNodes();
	};

}
#endif