#ifndef COMPONENTMODIFIED_H
#define COMPONENTMODIFIED_H
#include <entt.hpp>

namespace SliceEngine
{
	// Free functions invoked by entt registry when components are modified.
	void NotifyColliderShapeModified(entt::registry& reg, entt::entity entity);
	void NotifyRigidBodyModified(entt::registry& reg, entt::entity entity);
}


#endif