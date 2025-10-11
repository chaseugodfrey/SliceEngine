/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			ParticleSystemManager.cpp
 author:		Muhammad Hafiz Bin Onn
 email:			b.muhammadhafiz@digipen.edu
 brief:			Handles management of particle systems. 
				Manager > Particle Systems > Particles

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include <random>
#include "Systems/ParticleSystemManager.h"

namespace SliceEngine
{
#pragma region Manager Stuff
	void ParticleSystemManager::EntityOnInit(entt::registry& reg, entt::entity entity)
	{
		auto& ps = reg.get<ParticleSystem>(entity);
		ps.parentTransform = mRegistry->try_get<Transform>(entity);		
		InitializeSystem(ps);
	}
	void ParticleSystemManager::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{
		auto& ps = reg.get<ParticleSystem>(entity);
		UpdateSystem(ps,dt);
	}
	void ParticleSystemManager::EntityOnExit(entt::registry& reg, entt::entity entity)
	{
		auto& ps = reg.get<ParticleSystem>(entity);
		ExitSystem(ps);
	}
#pragma endregion

#pragma region System Stuff
	void ParticleSystemManager::InitializeSystem(ParticleSystem& ps)
	{
		ps.systemTimer = 0.0f;
		ps.particles.resize(ps.maxParticles);
		ps.oldestIndex = 0u;
		ps.awaitingIndex = 0u;

		gen.seed(ps.rd());
	}
	void ParticleSystemManager::UpdateSystem(ParticleSystem& ps, float dt)
	{
		ps.systemTimer += dt;

		// If system exceeded duration, flag as ending, if repeating, reset timer to dt
		if (ps.systemTimer >= ps.duration)
		{
			if (ps.isRepeating)
			{
				ps.systemTimer = dt;
			}
			else 
			{
				ps.systemEnding = true;
			}
		}

		// If system is not ending, activate new particles based on emission rate
		if (!ps.systemEnding)
		{
			ps.emissionAccumulator += (ps.emissionRate * dt);
			uint64_t particlesToSpawn = static_cast<uint64_t>(ps.emissionAccumulator);
			ps.emissionAccumulator -= static_cast<float>(particlesToSpawn);

			for (uint64_t i = 0; i < particlesToSpawn; ++i)
			{
				ActivateParticle(ps);				
			}
		}
	}
	void ParticleSystemManager::ExitSystem(ParticleSystem& ps)
	{

	}
#pragma endregion

#pragma region Particle Stuff

	void ParticleSystemManager::ActivateParticle(ParticleSystem& ps)
	{	
		if (ps.particles.empty())
		{
			return;
		}

		Particle& p = ps.particles[ps.awaitingIndex];
		p.active = true;
		p.age = 0.0f;

		InitializePosition(p,ps);
		InitializeRotation(p, ps);
		InitializeScale(p, ps);
		InitializeVelocity(p, ps);

		// Bounds checker
		ps.awaitingIndex = (ps.awaitingIndex + 1) % ps.particles.size();
	}
	void ParticleSystemManager::DeactivateParticle(uint64_t index, ParticleSystem& ps)
	{
		ps.particles[index].active = false;
	}

	void ParticleSystemManager::InitializePosition(Particle& p, ParticleSystem& ps)
	{
		if (ps.hasRandomSpawnPos)
		{
			// Create a distribution for each axis (x, y, z)
			std::uniform_real_distribution<float> distX(ps.minRandomSpawnPos.x, ps.maxRandomSpawnPos.x);
			std::uniform_real_distribution<float> distY(ps.minRandomSpawnPos.y, ps.maxRandomSpawnPos.y);
			std::uniform_real_distribution<float> distZ(ps.minRandomSpawnPos.z, ps.maxRandomSpawnPos.z);

			// Assign a random position within the range
			p.position = glm::vec3(distX(gen), distY(gen), distZ(gen));
		}
		else 
		{
			p.position = glm::vec3(0.0f);
		}

		if (ps.parentTransform)
		p.position += ps.parentTransform->position;
	}
	void ParticleSystemManager::InitializeRotation(Particle& p, ParticleSystem& ps)
	{
		if (ps.hasRandomInitialRotation)
		{
			// Interpolate between min and max quaternion
			std::uniform_real_distribution<float> tDist(0.0f, 1.0f);
			float t = tDist(gen);

			// Spherical linear interpolation between min and max rotations
			p.rotation = glm::slerp(ps.minRandomRotation, ps.maxRandomRotation, t);
		}
		else
		{
			p.rotation = ps.rotation;
		}
	}
	void ParticleSystemManager::InitializeScale(Particle& p, ParticleSystem& ps)
	{
		if (ps.hasRandomScale)
		{
			std::uniform_real_distribution<float> distX(ps.minRandomScale.x, ps.maxRandomScale.x);
			std::uniform_real_distribution<float> distY(ps.minRandomScale.y, ps.maxRandomScale.y);
			std::uniform_real_distribution<float> distZ(ps.minRandomScale.z, ps.maxRandomScale.z);

			p.scale = glm::vec3(distX(gen), distY(gen), distZ(gen));
		}
		else
		{
			p.scale = ps.scale;
		}
	}
	void ParticleSystemManager::InitializeVelocity(Particle& p, ParticleSystem& ps)
	{
		if (ps.hasRandomVelocity)
		{
			std::uniform_real_distribution<float> distX(ps.minRandomVelocity.x, ps.maxRandomVelocity.x);
			std::uniform_real_distribution<float> distY(ps.minRandomVelocity.y, ps.maxRandomVelocity.y);
			std::uniform_real_distribution<float> distZ(ps.minRandomVelocity.z, ps.maxRandomVelocity.z);

			p.velocity = glm::vec3(distX(gen), distY(gen), distZ(gen));
		}
		else
		{
			p.velocity = ps.velocity;
		}
	}
	void ParticleSystemManager::InitializeColour(Particle& p, ParticleSystem& ps)
	{
		if (ps.hasRandomColour)
		{
			std::uniform_real_distribution<float> distR(ps.minRandomColour.r, ps.maxRandomColour.r);
			std::uniform_real_distribution<float> distG(ps.minRandomColour.g, ps.maxRandomColour.g);
			std::uniform_real_distribution<float> distB(ps.minRandomColour.b, ps.maxRandomColour.b);
			std::uniform_real_distribution<float> distA(ps.minRandomColour.a, ps.maxRandomColour.a);

			p.colour = glm::vec4(distR(gen), distG(gen), distB(gen), distA(gen));
		}
		else
		{
			p.colour = ps.colour;
		}
	}

	void ParticleSystemManager::ApplyVeloctiy(ParticleSystem& ps, float dt)
	{

	}
	void ParticleSystemManager::ApplyGravity(ParticleSystem& ps, float dt)
	{

	}
	void ParticleSystemManager::ApplyCollision(ParticleSystem& ps, float dt)
	{
		//Idk
	}

#pragma endregion
}