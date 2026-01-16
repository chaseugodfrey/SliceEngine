/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			ComponentModified.h
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
#include "ComponentModified.h"

namespace SliceEngine
{
    //void NotifySliceEntityModified(entt::registry& reg, entt::entity entity)
    //{
    //    SliceEntityModifiedEvent event;
    //    event.entity = entity;

    //    EventManager::GetInstance()->Publish<SliceEntityModifiedEvent>(event);
    //}

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