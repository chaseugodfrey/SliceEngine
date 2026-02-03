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
#include "Helpers/Utilities.h"	

namespace SliceEngine 
{
	struct particleSystemEntity {};

	struct ParticleSystemManager : BaseSystem<particleSystemEntity, ParticleSystem>
	{
		static std::mt19937 gen;
		static std::random_device rd;

		std::vector<ParticleRenderPart> particlesTransforms;

		//Manager Layer
		void EntityOnEnter(entt::registry& reg, entt::entity entity) override;
		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override;
		void EntityOnExit(entt::registry& reg, entt::entity entity) override;

		void ResetManager();
		void ValidateParticleSystem(ParticleSystem& ps);
	private:
		// System Layer
		void InitializeSystem(ParticleSystem& ps);
		void UpdateSystem(ParticleSystem& ps, float dt);
		void ExitSystem(ParticleSystem& ps);
		void ResetSystem(ParticleSystem& ps, float dt);		

		// Particle Layer
		void ActivateParticle(ParticleSystem& ps);
		void DeactivateParticle(uint64_t index, ParticleSystem& ps);
		void DeactivateParticle(Particle& p, ParticleSystem& ps);
		void InitializeLifetime(Particle& p, ParticleSystem& ps);
		void InitializePosition(Particle& p, ParticleSystem& ps);
		void InitializeRotation(Particle& p, ParticleSystem& ps);
		void InitializeScale(Particle& p, ParticleSystem& ps);
		void InitializeVelocity(Particle& p, ParticleSystem& ps);
		void InitializeColour(Particle& p, ParticleSystem& ps);

		void ApplyVeloctiy(Particle& p, ParticleSystem& ps, float dt);
		void ApplyGravity(Particle& p, ParticleSystem& ps, float dt);
		void ApplyPhysics(Particle& p, ParticleSystem& ps, float dt);
		void ApplyColourOverLifetime(Particle& p, ParticleSystem& ps, float dt);

		void ApplyBurst(ParticleSystem& ps, float dt);

		glm::vec3 ComputeSphereInitialVelocity(const glm::vec3& center, const glm::vec3& position, float radius, float radialBias = 1.0f);
		glm::vec3 RandomDirectionInCone(float arcDegrees, ParticleSystem& ps);
		glm::vec3 RandomPointInSphere(float radius, float arcDegrees, ParticleSystem& ps);
		glm::vec3 RandomPointInCircle(float radius, ParticleSystem& ps);
	};
}


#endif
