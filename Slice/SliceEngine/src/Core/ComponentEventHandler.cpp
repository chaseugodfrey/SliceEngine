#include <pch.h>
#include "Core/EventManager.h"
#include <entt.hpp>
#include <unordered_map>
#include <rttr/variant.h>
#include "Events.h"

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
        //std::unordered_map<std::string, rttr::variant> properties;
        //properties["entity"] = static_cast<uint64_t>(entity);
        //EventManager::GetInstance()->Publish("ColliderShapeRemoved", properties);
    }

    void OnRigidBodyAdded(entt::registry& reg, entt::entity entity)
    {
        //std::unordered_map<std::string, rttr::variant> properties;
        //properties["entity"] = static_cast<uint64_t>(entity);
        //EventManager::GetInstance()->Publish("RigidBodyAdded", properties);
    }

    void OnRigidBodyRemoved(entt::registry& reg, entt::entity entity)
    {
        //std::unordered_map<std::string, rttr::variant> properties;
        //properties["entity"] = static_cast<uint64_t>(entity);
        //EventManager::GetInstance()->Publish("RigidBodyRemoved", properties);

    }

}