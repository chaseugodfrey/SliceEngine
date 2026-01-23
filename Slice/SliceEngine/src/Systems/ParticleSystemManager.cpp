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

		// Temp example having specific points of the curve to have certain colours
		if (ps.colourOverLifetime)
		{			
			ps.colourLifeTimeMap[0.0f] = ps.colour;
			ps.colourLifeTimeMap[1.0f] = ps.colourOverLifetimeEnd;
		}
	}
	void ParticleSystemManager::UpdateSystem(ParticleSystem& ps, float dt)
	{
		ValidateParticleSystem(ps);

		// If system exceeded duration, flag as ending, if repeating, reset timer to dt
		if (ps.systemTimer >= ps.duration)
		{
			if (ps.isRepeating)
			{
				ResetSystem(ps, dt);
			}
			else 
			{
				ps.systemTimer = ps.duration;
				ps.systemEnding = true;
			}
		}
		else 
		{
			ps.systemTimer += dt;
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
		if (ps.renderData.capacity() != ps.maxParticles)
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

			// handle final transform here
			glm::mat4 transformMatrix = glm::mat4x4(1.f);

			if (ps.isLocalSpace && ps.parentTransform)
			{
				transformMatrix = glm::translate(transformMatrix, p.position + ps.parentTransform->position);
			}
			else
			{
				transformMatrix = glm::translate(transformMatrix, p.position);
			}

			if (ps.isRotation3D)
			{
				prp.additionalRotation = p.rotation3D;
			}
			else 
			{
				prp.additionalRotation = glm::angleAxis(p.rotation, glm::vec3(0, 0, 1));
			}

			transformMatrix = glm::scale(transformMatrix, p.scale);

			prp.transform = transformMatrix;
			prp.textureID = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Texture>((GUID)ps.textureGUID.GetGUID()).get()->bindless_id;
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

	// Validates properties to prevent UDB or exceptions
	void ParticleSystemManager::ValidateParticleSystem(ParticleSystem& ps)
	{
		// Validate Scale
		Utilities::FixMinMax(ps.minRandomScale.x, ps.maxRandomScale.x);
		Utilities::FixMinMax(ps.minRandomScale.y, ps.maxRandomScale.y);
		Utilities::FixMinMax(ps.minRandomScale.z, ps.maxRandomScale.z);

		// Validate Lifetime
		Utilities::FixMinMax(ps.minParticleLifetime, ps.maxParticleLifetime);

		// Validate Rotation
		if (ps.isRotation3D)
		{
			Utilities::FixMinMax(ps.minRotation3DHint.x, ps.maxRotation3DHint.x);
			Utilities::FixMinMax(ps.minRotation3DHint.y, ps.maxRotation3DHint.y);
			Utilities::FixMinMax(ps.minRotation3DHint.z, ps.maxRotation3DHint.z);
		}
		else 
		{
			Utilities::FixMinMax(ps.minRandomRotation, ps.maxRandomRotation);
		}
		
		// Validate Start Position Offset
		Utilities::FixMinMax(ps.minRandomSpawnPos.x, ps.maxRandomSpawnPos.x);
		Utilities::FixMinMax(ps.minRandomSpawnPos.y, ps.maxRandomSpawnPos.y);
		Utilities::FixMinMax(ps.minRandomSpawnPos.z, ps.maxRandomSpawnPos.z);

		// Validate Colour
		Utilities::FixMinMax(ps.minRandomColour.r, ps.maxRandomColour.r);
		Utilities::FixMinMax(ps.minRandomColour.g, ps.maxRandomColour.g);
		Utilities::FixMinMax(ps.minRandomColour.b, ps.maxRandomColour.b);
		Utilities::FixMinMax(ps.minRandomColour.a, ps.maxRandomColour.a);

		// Validate Speed
		Utilities::FixMinMax(ps.minRandomSpeed, ps.maxRandomSpeed);

		// Validate Particle Pool
		if (ps.maxParticles != ps.particles.size())
		{
			ps.particles.resize(ps.maxParticles);
			ps.awaitingIndex = 0u;
		}

		// Validate Bursts
		if (ps.numBursts != ps.bursts.size())
		{
			ps.bursts.resize(ps.numBursts);
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
		if (ps.initialLifetimeType == ParticleSystem::TWO_CONSTANTS)
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

		switch (ps.shapeType)
		{
		case ParticleSystem::ShapeType::SPHERE:
			p.position += RandomPointInSphere(ps.sphereRadius);
			break;
		default:
			break;
		}
	}
	void ParticleSystemManager::InitializeRotation(Particle& p, ParticleSystem& ps)
	{
		if (ps.initialRotationType == ParticleSystem::ValueType::TWO_CONSTANTS)
		{
			if (ps.isRotation3D)
			{
				std::uniform_real_distribution<float> distX(ps.minRotation3DHint.x, ps.maxRotation3DHint.x);
				std::uniform_real_distribution<float> distY(ps.minRotation3DHint.y, ps.maxRotation3DHint.y);
				std::uniform_real_distribution<float> distZ(ps.minRotation3DHint.z, ps.maxRotation3DHint.z);

				glm::vec3 eul{
					distX(gen),
					distY(gen),
					distZ(gen)
				};
				p.rotation3D = glm::quat(glm::radians(eul));
			}
			else 
			{
				std::uniform_real_distribution<float> dist(
					ps.minRandomRotation,
					ps.maxRandomRotation
				);

				p.rotation = dist(gen);
			}
		}
		else
		{
			if (ps.isRotation3D)
			{
				p.rotation3D = glm::quat(glm::radians(ps.rotation3DHint));
			}
			else 
			{
				p.rotation = ps.rotation;
			}
		}
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
		if (ps.speedValueType == ParticleSystem::ValueType::TWO_CONSTANTS)
		{
			std::uniform_real_distribution<float> dist(
				ps.minRandomSpeed,
				ps.maxRandomSpeed);

			p.speed = dist(gen);
		}
		else
		{
			p.speed = ps.speed;
		}

		glm::vec3 direction(0.0f);

		switch (ps.shapeType)
		{
		case ParticleSystem::ShapeType::SPHERE:
			direction = ComputeSphereInitialVelocity(ps.parentTransform->position, p.position, ps.sphereRadius);
			break;
		default:
			direction = glm::vec3(0.0f, 1.0f, 0.0f); // fallback
			break;
		}

		p.velocity = glm::normalize(direction) * p.speed;
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
		// May need to remove in future
		ps.colourLifeTimeMap[0.0f] = ps.colour;
	}

	void ParticleSystemManager::ApplyVeloctiy(Particle& p, ParticleSystem& ps, float dt)
	{
		p.position += p.velocity * dt;
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
		auto& map = ps.colourLifeTimeMap;

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

	glm::vec3 ParticleSystemManager::ComputeSphereInitialVelocity(
		const glm::vec3& center,
		const glm::vec3& position,
		float radius,
		float radialBias
		)
	{
		std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

		// Direction from center to particle
		glm::vec3 dir = glm::normalize(position - center);

		radialBias = glm::clamp(radialBias, 0.0f, 1.0f);

		if (glm::dot(dir, dir) < 1e-6f)
			dir = glm::vec3(0.0f, 1.0f, 0.0f);// fallback
		else
			dir = glm::normalize(dir);

		// Random unit vector for spread
		glm::vec3 randDir;
		do {
			randDir = glm::vec3(
				dist01(gen) * 2.0f - 1.0f,
				dist01(gen) * 2.0f - 1.0f,
				dist01(gen) * 2.0f - 1.0f
			);
		} while (glm::dot(randDir, randDir) < 1e-6f);
		randDir = glm::normalize(randDir);

		// Blend radial + random
		dir = glm::normalize(glm::mix(randDir, dir, radialBias));

		return dir;
	}

	glm::vec3 ParticleSystemManager::RandomPointInSphere(float radius)
	{
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);

		float u = dist(gen);
		float v = dist(gen);
		float w = dist(gen);

		// Random direction
		float theta = 2.0f * glm::pi<float>() * u;
		float phi = acos(2.0f * v - 1.0f);

		float sinPhi = sin(phi);

		glm::vec3 dir(
			cos(theta) * sinPhi,
			sin(theta) * sinPhi,
			cos(phi)
		);

		// Correct radial distribution
		float r = radius * cbrt(w);

		return dir * r;
	}
#pragma endregion
}