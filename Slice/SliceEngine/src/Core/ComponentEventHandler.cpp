#include <pch.h>
#include "Core/EventManager.h"
#include <entt.hpp>
#include <unordered_map>
#include <rttr/variant.h>
#include "Events.h"
#include "ComponentEventHandler.h"

namespace SliceEngine
{


    void OnColliderShapeAdded(entt::registry& reg, entt::entity entity)
    {
        ColliderShapeAddedEvent event;
		event.entity = entity;
       
        EventManager::GetInstance()->Publish<ColliderShapeAddedEvent>(event);
    }

    void OnColliderShapeRemoved(entt::registry& reg, entt::entity entity)
    {
        ColliderShapeRemovedEvent event;
        event.entity = entity;

        EventManager::GetInstance()->Publish<ColliderShapeRemovedEvent>(event);
    }

    void OnRigidBodyAdded(entt::registry& reg, entt::entity entity)
    {
        RigidBodyAddedEvent event;
        event.entity = entity;

        EventManager::GetInstance()->Publish<RigidBodyAddedEvent>(event);
    }

    void OnRigidBodyRemoved(entt::registry& reg, entt::entity entity)
    {
        RigidBodyRemovedEvent event;
        event.entity = entity;

        EventManager::GetInstance()->Publish<RigidBodyRemovedEvent>(event);

    }

}