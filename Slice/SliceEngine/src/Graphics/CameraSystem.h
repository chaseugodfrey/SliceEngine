/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			CameraSystem.h
 author:		Won Yu Xuan Rainne
 email:			won.m@digipen.edu
 brief:			Handles Creation of individual render targets for each camera

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef CAMERA_SYSTEM_H
#define CAMERA_SYSTEM_H

#include "../ECS/BaseSystem.h"
#include "../ECS/ECSTypes.h"

namespace SliceEngine
{
	struct cameraEntity {};

	struct CameraSystem : BaseSystem<cameraEntity, Transform, Camera>
	{
		void EntityOnEnter(entt::registry& reg, entt::entity entity) override;
		void EntityOnExit(entt::registry& reg, entt::entity entity) override;
		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override;
		const int maxHeight{ 1080 }, maxWidth{ 1920 };

		std::optional<Entity> mainCam;
		std::vector<Entity> gameCameras;
	};
}

#endif