/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        SoundSystem.h

 author:	  Lee Yong Yee

 email:       l.yongyee@digipen.edu

 brief:		  Declares the SoundSystem class, which integrates with the ECS to manage audio 
              components (AudioSource) in the engine using the FMOD sound library. 
              The system handles entity lifecycle events (enter, exit, update), synchronizes 
              component state with FMOD, and supports playback control such as pause, volume, 
              looping, and positional audio for 2D and 3D sounds.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef SOUND_SYSTEM_H
#define SOUND_SYSTEM_H

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

		/**
			* @brief Binds FMOD resources to all AudioSource components.
			*
			*/
		void BindToAudioSource();

		/**
			* @brief Called when an entity with an AudioSource enters the system.
			*
			* @param reg    Reference to the ECS registry.
			* @param entity The entity that entered.
			*/
		void EntityOnEnter(entt::registry& reg, entt::entity entity) override;

		/**
			* @brief Called when an entity with an AudioSource exits the system.
			*
			* @param reg    Reference to the ECS registry.
			* @param entity The entity that exited.
			*
			*/
		void EntityOnExit(entt::registry& reg, entt::entity entity) override;

		/**
		* @brief Updates an entity’s AudioSource component each frame.
		*
		* @param reg    Reference to the ECS registry.
		* @param entity The entity to update.
		* @param dt     Delta time since the last update, in seconds.
		*/
		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override;

		/**
		* @brief Updates the FMOD state for a specific entity’s AudioSource.
		*
		* @param reg    Reference to the ECS registry.
		* @param entity The entity to update.
		*/
		void ComponentUpdate(entt::registry& reg, entt::entity entity);

		/**
		 * @brief Reacts when an AudioSource’s pause state changes.
		 *
		 * @param reg    Reference to the ECS registry.
		 * @param entity The entity whose pause state was updated.
		 */
		void onPauseUpdated(entt::registry& reg, entt::entity);
	};
}

#endif