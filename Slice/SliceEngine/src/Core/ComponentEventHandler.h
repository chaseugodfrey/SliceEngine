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
}


#endif