/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			ComponentEventHandler.cpp
 author:		Gideon Francis
 email:			g.francis@digipen.edu
 brief:			Handles component events

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
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

    //void OnColliderShapeRemoved(entt::registry& reg, entt::entity entity)
    //{
    //    ColliderShapeRemovedEvent event;
    //    event.entity = entity;

    //    EventManager::GetInstance()->Publish<ColliderShapeRemovedEvent>(event);
    //}

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

    void OnNetworkClientConnect(std::string ip, std::string port)
    {
        NetworkClientConnectEvent event;
        event.ip = ip;
        event.port = port;

        EventManager::GetInstance()->Publish<NetworkClientConnectEvent>(event);
    }

    void OnNetworkBindPort(std::string port)
    {
        NetworkBindPortEvent event;
        event.port = port;

        EventManager::GetInstance()->Publish<NetworkBindPortEvent>(event);
    }

    void OnGONetworkEvent(Entity entity, bool create)
    {
        GONetworkEvent event;
        event.entity = entity;
        event.create = create;

        EventManager::GetInstance()->Publish<GONetworkEvent>(event);
    }
}