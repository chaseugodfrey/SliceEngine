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
	struct particleSystemEntity {};

	struct ParticleSystemManager : BaseSystem<particleSystemEntity, ParticleSystem>
	{
		static std::mt19937 gen;
		static std::random_device rd;

		//Manager Layer
		void EntityOnEnter(entt::registry& reg, entt::entity entity) override;
		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override;
		void EntityOnExit(entt::registry& reg, entt::entity entity) override;

	private:
		// System Layer
		void InitializeSystem(ParticleSystem& ps);
		void UpdateSystem(ParticleSystem& ps, float dt);
		void ExitSystem(ParticleSystem& ps);
		void ResetSystem(ParticleSystem& ps, float dt);

		// Particle Layer
		void ActivateParticle(ParticleSystem& ps);
		void DeactivateParticle(uint64_t index, ParticleSystem& ps);

		void InitializePosition(Particle& p, ParticleSystem& ps);
		void InitializeRotation(Particle& p, ParticleSystem& ps);
		void InitializeScale(Particle& p, ParticleSystem& ps);
		void InitializeVelocity(Particle& p, ParticleSystem& ps);
		void InitializeColour(Particle& p, ParticleSystem& ps);

		void ApplyVeloctiy(Particle& p, ParticleSystem& ps, float dt);
		void ApplyGravity(Particle& p, ParticleSystem& ps, float dt);
		void ApplyCollision(Particle& p, ParticleSystem& ps, float dt);
		void ApplyBurst(ParticleSystem& ps, float dt);

	// Tests
	public:

		// Init Test
		void Test1();
		
		// Update Test
		void Test2Init();
		void Test2Update();
	};
}



#endif
