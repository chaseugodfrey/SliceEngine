/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			BoneSystem.h
 author:		Elton leosantosa
 email:			leosantosa@digipen.edu
 brief:			System that handles updating of scenegraph transforms from animated skeleton bones

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef BONE_SYSTEM_H
#define	BONE_SYSTEM_H

#include "../ECS/BaseSystem.h"
#include "../ECS/ECSTypes.h"

namespace SliceEngine
{
	//include the real one once done
	struct Bone_Entity {};

	class BoneSystem : public BaseSystem<Bone_Entity, Bone, Transform>
	{
	public:
		void EntityOnEnter(entt::registry& reg, entt::entity entity) override {};
		void EntityOnExit(entt::registry& reg, entt::entity entity) override {};
		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override {};
		void Update_Scenegraph() const;
	};

	

}

#endif
