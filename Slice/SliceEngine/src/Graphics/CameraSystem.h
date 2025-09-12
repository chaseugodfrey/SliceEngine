#ifndef CAMERA_SYSTEM_H
#define CAMERA_SYSTEM_H

#include "../ECS/BaseSystem.h"
#include "../ECS/ECSTypes.h"
#include <optional>

namespace SliceEngine
{
	struct cameraEntity {};

	struct CameraSystem : BaseSystem<cameraEntity, Transform, Camera>
	{
		void EntityOnEnter(entt::registry& reg, entt::entity entity) override;
		void EntityOnExit(entt::registry& reg, entt::entity entity) override;
		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override;

		// TODO: Please change this
		const int GAMESCREENWIDTH{900}, GAMESCREENHEIGHT{600};
	};
}

#endif