#include <pch.h>
#include "SessionManager.h"
#include <Core/EventManager.h>

namespace SliceEditor
{
	SessionManager::SessionManager(Registry& reg) : IBaseManager(reg) {}

	SessionManager::~SessionManager() {}

	void SessionManager::Init()
	{
		auto* eventManager = EventManager::GetInstance();

		eventManager->Subscribe<OnSceneLoadedEvent, &SessionManager::OnSceneChange>(this);
	}

	void SessionManager::Update()
	{
		CreateEntityNodes();
	}


	void SessionManager::CreateEntityNodes()
	{
		auto view = SliceEngine::Core::GetInstance()->GetRegistry().view<SliceEngine::SceneGraph>();

		if (view.size() != mEntityNodes.size())
		{
			mEntityNodes.clear();
			for (auto entity : view)
			{
				mEntityNodes.emplace(entity, std::make_unique<EntityNode>(entity));
			}
		}
	}

	void SessionManager::OnSceneChange(const OnSceneLoadedEvent& event)
	{
		if (event.isSceneLoaded)
		{
			mEntityNodes.clear();
			CreateEntityNodes();
		}
	}

	std::unordered_map<entt::entity, std::unique_ptr<EntityNode>>& SessionManager::GetEntityNodes()
	{
		return mEntityNodes;
	}



}
