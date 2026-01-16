/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			ComponentModified.h
 author:		Gideon Francis
 email:			g.francis@digipen.edu
 brief:			Handles component events

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef COMPONENTMODIFIED_H
#define COMPONENTMODIFIED_H
#include <entt.hpp>

namespace SliceEngine
{
	// Free functions invoked by entt registry when components are modified.
	//void NotifySliceEntityModified(entt::registry& reg, entt::entity entity);
	void NotifyColliderShapeModified(entt::registry& reg, entt::entity entity);
	void NotifyRigidBodyModified(entt::registry& reg, entt::entity entity);
}


#endif