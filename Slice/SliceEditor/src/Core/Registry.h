/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        Registry.h

 author:	  Chase Rodgrigues

 email:       rodrigues.i@digipen.edu

 brief:		  Declares the Registry class, which holds all the managers of the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef EDITOR_REGISTRY_H
#define EDITOR_REGISTRY_H

#include "IBaseManager.h"
#include "../AssetManager/AssetManager.h"
#include "Systems/FramerateManager.h"

namespace SliceEditor
{
	class Registry
	{
		std::unordered_map<std::string, std::unique_ptr<IBaseManager>> mManagers;
		AssetManager& mAssetManager;

		template <typename ManagerType>
		void CreateManager(std::string name)
		{
			static_assert(std::is_base_of_v<IBaseManager, ManagerType>, "ManagerType must derive from IBaseManager");
			auto manager = std::make_unique<ManagerType>(*this);
			mManagers.emplace(name, std::move(manager));
		}

	public:

		Registry(AssetManager& ass) : mAssetManager(ass)
		{ }

		inline AssetManager& GetAssetManager()
		{
			return mAssetManager;
		}

		void Init();
		void Update();

		std::unordered_map<std::string, std::unique_ptr<IBaseManager>> const& GetManagers();

		template <typename IManager>
		IManager* GetManager(std::string_view name)
		{
			auto it = mManagers.find(std::string(name));
			if (it != mManagers.end())
				return static_cast<IManager*>(it->second.get());
			return nullptr;
		}
	};
}

#endif