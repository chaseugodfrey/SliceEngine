#ifndef SOUND_SYSTEM_H
#define SOUND_SYSTEM_H

#include <pch.h>
#include "ECS/BaseSystem.h"
#include "ECS/ECSTypes.h"
#include "../Core/Core.h"
#include <iostream>
#include <string>
// for testing purpose


namespace SliceEngine
{

	// for keeping track of entities that belong to sound system
	struct SoundEntity {};

	struct SoundSystem : BaseSystem<SoundEntity, AudioSource>
	{
		

		public:

		void BindToAudioSource();

		void EntityOnEnter(entt::registry& reg, entt::entity entity) override;

		void EntityOnExit(entt::registry& reg, entt::entity entity) override;
		

		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override;
		
		void onVolumeUpdated(entt::registry& reg, entt::entity);

		void onPauseUpdated(entt::registry& reg, entt::entity);
	};
}

#endif