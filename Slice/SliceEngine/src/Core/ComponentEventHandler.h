/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			ComponentEventHandler.h
 author:		Gideon Francis
 email:			g.francis@digipen.edu
 brief:			Handles component events

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef COMPONENT_EVENT_HANDLER_H
#define COMPONENT_EVENT_HANDLER_H
#include <entt.hpp>

namespace SliceEngine
{
	void OnColliderShapeAdded(entt::registry& reg, entt::entity entity);
	void OnColliderShapeRemoved(entt::registry& reg, entt::entity entity);
	void OnRigidBodyAdded(entt::registry& reg, entt::entity entity);
	void OnRigidBodyRemoved(entt::registry& reg, entt::entity entity);
	void OnNetworkClientConnect(std::string ip, std::string port);
	void OnNetworkBindPort(std::string port);
	void OnGONetworkEvent(Entity entity, bool create);
}


#endif