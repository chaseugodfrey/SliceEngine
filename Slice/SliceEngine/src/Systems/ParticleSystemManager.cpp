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
#include "../Serializer/JSONSerializer.h"
#include <Core/Core.h>

namespace SliceEngine
{
#pragma region Manager Stuff
	std::mt19937 ParticleSystemManager::gen{ std::random_device{}() };
	void ParticleSystemManager::EntityOnEnter(entt::registry& reg, entt::entity entity)
	{
		auto& ps = reg.get<ParticleSystem>(entity);
		ps.parentTransform = mRegistry->try_get<Transform>(entity);
		particlesTransforms.reserve(std::numeric_limits<uint16_t>::max());
		InitializeSystem(ps);
	}
	void ParticleSystemManager::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{		
		auto& ps = reg.get<ParticleSystem>(entity);
		ps.parentTransform = mRegistry->try_get<Transform>(entity);
		if (ps.expired || (!ps.isActive))
		{
			if (ps.destroyOnExpire)
			{
				FactoryInstance.Destroy(entity);
			}
			else 
			{
				ps.isActive = false;
			}
			return;
		}
		UpdateSystem(ps,dt);
	}
	void ParticleSystemManager::EntityOnExit(entt::registry& reg, entt::entity entity)
	{
		auto& ps = reg.get<ParticleSystem>(entity);
		ExitSystem(ps);
	}

	void ParticleSystemManager::ResetManager()
	{
	}

#pragma endregion

#pragma region System Stuff
	void ParticleSystemManager::InitializeSystem(ParticleSystem& ps)
	{
		ps.systemTimer = 0.0f;
		ps.particles.resize(ps.maxParticles);
		ps.oldestIndex = 0u;
		ps.awaitingIndex = 0u;

		try 
		{
			ps.renderData.reserve(ps.maxParticles);
		}
		catch (const std::bad_alloc&) 
		{
			std::cerr << "Allocation failed!" << std::endl;
		}
	}
	void ParticleSystemManager::UpdateSystem(ParticleSystem& ps, float dt)
	{
		ps.systemTimer += dt;

		// If system exceeded duration, flag as ending, if repeating, reset timer to dt
		if (ps.systemTimer >= ps.duration)
		{
			if (ps.isRepeating)
			{
				ResetSystem(ps, dt);
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

		if (!ps.bursts.empty())
		{
			ApplyBurst(ps, dt);
		}

		bool haveActiveParticle = false;
		// Update all particles to get final transform
		for (Particle& p : ps.particles)
		{
			if (!p.active)
			{
				continue;
			}
			else 
			{
				haveActiveParticle = true;
			}

			p.age += dt;
			if (p.age > p.maxAge)
			{
				DeactivateParticle(p,ps);
				continue;
			}

			ApplyVeloctiy(p, ps, dt);

			if (ps.gForce != 0.0f)
			{
				ApplyGravity(p, ps, dt);
			}

			if (ps.hasCollision)
			{
				ApplyCollision(p, ps, dt);
			}

			if (ps.colourOverLifetime)
			{
				ApplyColourOverLifetime(p, ps, dt);
			}
		}
		if (!haveActiveParticle && ps.systemEnding)
		{
			ps.expired = true;
		}

		// Get all particles' final transforms to be renderered
		ps.renderData.clear();
		if (ps.renderData.size() != ps.maxParticles)
		{
			ps.renderData.reserve(ps.maxParticles);
		}

		for (Particle& p : ps.particles)
		{
			if (!p.active)
			{
				continue;
			}

			ParticleRenderPart prp;
			glm::mat4 Rot;

			// handle final transform here
			glm::mat4 transformMatrix = glm::mat4x4(1.f);

			if (ps.isLocalSpace && ps.parentTransform)
			{
				transformMatrix = glm::translate(transformMatrix, p.position);
				Rot = glm::mat4_cast(p.rotation + ps.parentTransform->rotation);
				transformMatrix *= Rot;
				transformMatrix = glm::scale(transformMatrix, p.scale);
			}
			else
			{
				transformMatrix = glm::translate(transformMatrix, p.position);
				Rot = glm::mat4_cast(p.rotation);
				transformMatrix *= Rot;
				transformMatrix = glm::scale(transformMatrix, p.scale);
			}

			//glm::mat4x4 Rot = glm::eulerAngleXYZ(glm::radians(transform.rotation.x), glm::radians(transform.rotation.y + 90.f), glm::radians(transform.rotation.z));

			prp.transform = transformMatrix;
			prp.textureID = ps.GetTextureID();
			prp.colour = p.colour;

			ps.renderData.push_back(prp);
		}
		particlesTransforms.insert(particlesTransforms.end(), ps.renderData.begin(), ps.renderData.end());
	}
	void ParticleSystemManager::ExitSystem(ParticleSystem& ps)
	{

	}

	void ParticleSystemManager::ResetSystem(ParticleSystem& ps, float dt)
	{
		ps.systemTimer = dt;

		for (ParticleSystem::Burst& b : ps.bursts)
		{
			b.triggered = false;
		}
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

		InitializeLifetime(p, ps);
		InitializePosition(p,ps);
		InitializeRotation(p, ps);
		InitializeScale(p, ps);
		InitializeVelocity(p, ps);
		InitializeColour(p, ps);

		// Bounds checker
		ps.awaitingIndex = (ps.awaitingIndex + 1) % ps.particles.size();
	}
	void ParticleSystemManager::DeactivateParticle(uint64_t index, ParticleSystem& ps)
	{
		ps.particles[index].active = false;
	}

	void ParticleSystemManager::DeactivateParticle(Particle& p, ParticleSystem& ps)
	{
		p.active = false;
	}

	void ParticleSystemManager::InitializeLifetime(Particle& p, ParticleSystem& ps)
	{
		p.age = 0.0f;
		if (ps.colourOverLifetime)
		{
			std::uniform_real_distribution<float> randAge(ps.minParticleLifetime, ps.maxParticleLifetime);
			p.maxAge = randAge(gen);
		}
		else 
		{
			p.maxAge = ps.lifetime;
		}
	}

	void ParticleSystemManager::InitializePosition(Particle& p, ParticleSystem& ps)
	{
		if (ps.posValueType == ParticleSystem::TWO_CONSTANTS)
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
			p.position = ps.spawnPos;
		}

		if (ps.parentTransform)
		p.position += ps.parentTransform->position;
	}
	void ParticleSystemManager::InitializeRotation(Particle& p, ParticleSystem& ps)
	{
		if (ps.initialRotationType == ParticleSystem::ValueType::TWO_CONSTANTS)
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

		if (ps.parentTransform)
		p.rotation = ps.parentTransform->rotation * p.rotation;
	}
	void ParticleSystemManager::InitializeScale(Particle& p, ParticleSystem& ps)
	{
		if (ps.scaleType == ParticleSystem::ValueType::TWO_CONSTANTS)
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
		
		// Should particles inherit the scale of its parent? no right?
		// if (ps.parentTransform)
		// p.scale += ps.parentTransform->scale;
	}
	void ParticleSystemManager::InitializeVelocity(Particle& p, ParticleSystem& ps)
	{
		if (ps.velocityValueType == ParticleSystem::ValueType::TWO_CONSTANTS)
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
		if (ps.colourValueType == ParticleSystem::ValueType::TWO_CONSTANTS)
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

	void ParticleSystemManager::ApplyVeloctiy(Particle& p, ParticleSystem& ps, float dt)
	{
		glm::vec3 worldVelocity = p.rotation * p.velocity;
		p.position += worldVelocity * ps.speed * dt;
	}

	void ParticleSystemManager::ApplyGravity(Particle& p, ParticleSystem& ps, float dt)
	{
		p.velocity += glm::vec3(0.0f, -(ps.gForce * dt), 0.0f);
	}
	void ParticleSystemManager::ApplyCollision(Particle& p, ParticleSystem& ps, float dt)
	{
		//Idk
	}
	void ParticleSystemManager::ApplyBurst(ParticleSystem& ps, float dt)
	{
		for (ParticleSystem::Burst& b : ps.bursts)
		{
			if (!b.triggered && ps.systemTimer >= b.triggerTime)
			{
				b.triggered = true;
				b.burstRepetitions = std::max<uint64_t>(1, b.burstRepetitions); // ensure at least 1
				b.repTimer = 0.0f; // optional helper field
				b.repsDone = 0; // optional helper field
			}

			if (b.triggered && b.repsDone < b.burstRepetitions)
			{
				b.repTimer += dt;

				if (b.repTimer >= b.burstPeriod)
				{
					// Spawn particles for this repetition
					for (uint64_t i = 0; i < b.numParticles; ++i)
					{
						ActivateParticle(ps);
					}

					b.repTimer -= b.burstPeriod;
					++b.repsDone;
				}
			}
		}
	}

	void ParticleSystemManager::ApplyColourOverLifetime(Particle& p, ParticleSystem& ps, float dt)
	{
		auto& map = ps.colorLifeTimeMap;

		if (map.empty())
			return;		

		float t = glm::clamp(p.normalizedLifetime(), 0.0f, 1.0f);

		// If only one color, just use it
		if (map.size() == 1)
		{
			p.colour = map.begin()->second;
			return;
		}

		// First keyframe with key > t
		auto upper = map.upper_bound(t);

		// If t is before the first key
		if (upper == map.begin())
		{
			p.colour = upper->second;
			return;
		}

		// If t is after the last key
		if (upper == map.end())
		{
			p.colour = std::prev(upper)->second;
			return;
		}

		// Interpolate between lower and upper
		auto lower = std::prev(upper);

		float t0 = lower->first;
		float t1 = upper->first;

		const glm::vec4& c0 = lower->second;
		const glm::vec4& c1 = upper->second;

		float localT = (t - t0) / (t1 - t0);

		p.colour = glm::mix(c0, c1, localT);
	}
#pragma endregion
}