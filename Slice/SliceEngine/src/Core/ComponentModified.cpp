#include <pch.h>
#include "Core/EventManager.h"
#include <entt.hpp>
#include <unordered_map>
#include <rttr/variant.h>
#include "Events.h"
#include "ComponentModified.h"

namespace SliceEngine
{


    void NotifyColliderShapeModified(entt::registry& reg, entt::entity entity)
    {
        ColliderShapeModifiedEvent event;
        event.entity = entity;

        EventManager::GetInstance()->Publish<ColliderShapeModifiedEvent>(event);
    }

    void NotifyRigidBodyModified(entt::registry& reg, entt::entity entity)
    {
        RigidBodyModifiedEvent event;
        event.entity = entity;

        EventManager::GetInstance()->Publish<RigidBodyModifiedEvent>(event);
    }

}