#ifndef EDITOR_REGISTRY_H
#define EDITOR_REGISTRY_H

#include "IBaseManager.h"
#include "SelectionSystem/SelectionSystem.h"

namespace SliceEditor
{
	class Registry
	{
		SelectionSystem selectionSystem;

		std::unordered_map<std::string, std::unique_ptr<IBaseManager>> mManagers;

		template <typename ManagerType>
		void CreateManager(std::string name)
		{
			static_assert(std::is_base_of_v<IBaseManager, ManagerType>, "ManagerType must derive from IBaseManager");
			auto manager = std::make_unique<ManagerType>(*this);
			mManagers.emplace(name, std::move(manager));
		}

	public:

		void Init();

		std::unordered_map<std::string, std::unique_ptr<IBaseManager>> const& GetManagers();

		// To do:: bring this to a system map
		SelectionSystem& GetSelectionSystem();

		template <typename IManager>
		IManager* GetManager(std::string_view name)
		{
			auto it = mManagers.find(std::string(name));
			if (it != mManagers.end())
				return dynamic_cast<IManager*>(it->second.get());
			return nullptr;
		}

	};
}

#endif