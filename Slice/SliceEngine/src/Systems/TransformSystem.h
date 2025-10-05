/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			TransformSystem.h
 author:		Chase Roderigues
 email:			roderigues.i@digipen.edu
 brief:			Handles Transformations of Game Objects

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef TRANSFORM_MANAGER_H
#define TRANSFORM_MANAGER_H

#include "ECS/BaseSystem.h"
#include "ECS/ECSTypes.h"

namespace SliceEngine
{
	struct transformEntity {};

	struct TransformSystem : BaseSystem<transformEntity, Transform>
	{
		void EntityOnEnter(entt::registry& reg, entt::entity entity) override;
		void EntityOnExit(entt::registry& reg, entt::entity entity) override;
		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override;
	};
}


#endif