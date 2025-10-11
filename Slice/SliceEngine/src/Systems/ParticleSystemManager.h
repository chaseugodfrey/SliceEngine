/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			ParticleSystemManager.h
 author:		Muhammad Hafiz Bin Onn
 email:			b.muhammadhafiz@digipen.edu
 brief:			Handles management of particle systems.
				Manager > Particle Systems > Particles

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef PARTICLE_SYSTEM_MANAGER_H
#define PARTICLE_SYSTEM_MANAGER_H

#include "ECS/BaseSystem.h"
#include "ECS/ECSTypes.h"

namespace SliceEngine 
{
	struct ParticleSystemManager : BaseSystem<ParticleSystem>
	{
		static std::mt19937 gen;

		//Manager Layer
		void EntityOnInit(entt::registry& reg, entt::entity entity);
		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt);
		void EntityOnExit(entt::registry& reg, entt::entity entity);

	private:
		// System Layer
		void InitializeSystem(ParticleSystem& ps);
		void UpdateSystem(ParticleSystem& ps, float dt);
		void ExitSystem(ParticleSystem& ps);

		// Particle Layer
		void ActivateParticle(ParticleSystem& ps);
		void DeactivateParticle(uint64_t index, ParticleSystem& ps);

		void InitializePosition(Particle& p, ParticleSystem& ps);
		void InitializeRotation(Particle& p, ParticleSystem& ps);
		void InitializeScale(Particle& p, ParticleSystem& ps);
		void InitializeVelocity(Particle& p, ParticleSystem& ps);
		void InitializeColour(Particle& p, ParticleSystem& ps);

		void ApplyVeloctiy(ParticleSystem& ps, float dt);
		void ApplyGravity(ParticleSystem& ps, float dt);
		void ApplyCollision(ParticleSystem& ps, float dt);
	};
}



#endif
