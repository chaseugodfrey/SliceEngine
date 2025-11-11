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
	std::random_device ParticleSystemManager::rd{};
	std::mt19937 ParticleSystemManager::gen{ rd() };

	void ParticleSystemManager::EntityOnEnter(entt::registry& reg, entt::entity entity)
	{
		auto& ps = reg.get<ParticleSystem>(entity);
		ps.parentTransform = mRegistry->try_get<Transform>(entity);	
		InitializeSystem(ps);
	}
	void ParticleSystemManager::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{
		auto& ps = reg.get<ParticleSystem>(entity);
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
#pragma endregion

#pragma region System Stuff
	void ParticleSystemManager::InitializeSystem(ParticleSystem& ps)
	{
		ps.systemTimer = 0.0f;
		ps.particles.resize(ps.maxParticles);
		ps.oldestIndex = 0u;
		ps.awaitingIndex = 0u;		

		gen.seed(rd());

		try 
		{
			particlesTransforms.reserve(std::numeric_limits<uint16_t>::max());
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

		if (ps.hasBursts)
		{
			ApplyBurst(ps, dt);
		}

		bool isAnyParticleActive = false;
		for (Particle& p : ps.particles)
		{
			if (p.active)
			{
				isAnyParticleActive = true;

				ApplyVeloctiy(p, ps, dt);

				if (ps.hasGravity)
				{
					ApplyGravity(p, ps, dt);
				}

				if (ps.hasCollision)
				{
					ApplyCollision(p, ps, dt);
				}
			}
		}
		if (!isAnyParticleActive && ps.systemEnding)
		{
			ps.expired = true;
		}

		particlesTransforms.clear();

		for (auto& particle : ps.particles)
		{
			if (particle.active)
			{
				ParticleRenderPart prp;
				glm::mat4 Rot;

				// handle final transform here
				glm::mat4 transformMatrix = glm::mat4x4(1.f);

				if (ps.isLocalSpace && ps.parentTransform)
				{
					transformMatrix = glm::translate(transformMatrix, particle.position + ps.parentTransform->position);
					Rot = glm::mat4_cast(particle.rotation + ps.parentTransform->rotation);
					transformMatrix *= Rot;
					transformMatrix = glm::scale(transformMatrix, particle.scale);
				}
				else 
				{
					transformMatrix = glm::translate(transformMatrix, particle.position);
					Rot = glm::mat4_cast(particle.rotation);
					transformMatrix *= Rot;
					transformMatrix = glm::scale(transformMatrix, particle.scale);
				}

				//glm::mat4x4 Rot = glm::eulerAngleXYZ(glm::radians(transform.rotation.x), glm::radians(transform.rotation.y + 90.f), glm::radians(transform.rotation.z));
						
				prp.transform = transformMatrix;				
				prp.textureID = ps.textureID;
				prp.colour = ps.colour;

				particlesTransforms.push_back(prp);
			}
		}

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
		p.age = 0.0f;

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

		if (ps.parentTransform)
		p.rotation += ps.parentTransform->rotation;
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
		
		// Should particles inherit the scale of its parent? no right?
		// if (ps.parentTransform)
		// p.scale += ps.parentTransform->scale;
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

	void ParticleSystemManager::ApplyParentTransform(Particle& p, ParticleSystem& ps)
	{

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
#pragma endregion

#pragma region Tests	
	void ParticleSystemManager::Test1()
	{
		SLICE_LOG("Test 1 Beginning...");
		auto& factory = FactoryInstance;
		GameObject roy = factory.CreateGO("ParticleSystemTest");

		SLICE_LOG("Adding Particle System Component...");
		roy.AddComponent<ParticleSystem>();

		json output = SliceEngine::JSONSerializer::SerializeGameObject(roy);
		SLICE_LOG(output.dump(4));

		SLICE_LOG("Deleting Test 1's gameobject...");
		factory.Destroy(roy);

		SLICE_LOG("Test 1 Ended.");
	}

	void ParticleSystemManager::Test2Init()
	{
		SLICE_LOG("Test 2 Beginning...");

		auto& factory = FactoryInstance;
		factory.CreateGO("ParticleSystemTest");
		GameObject roy = factory.GetGOByName("ParticleSystemTest");

		SLICE_LOG("Adding Particle System Component...");
		roy.AddComponent<ParticleSystem>();

		SLICE_LOG("Modifying base values for simulation...");
		auto& ps = roy.GetComponent<ParticleSystem>();
		ps.duration = 1.0f;
		ps.velocity = glm::vec3(1.0f, 1.0f, 0.0f);
		ps.emissionRate = 5.0f;
		ps.lifetime = 2.0f;
		ps.destroyOnExpire = true;
	}

	void ParticleSystemManager::Test2Update()
	{
		bool testEnded{ false };
		
		if (testEnded)
		{
			return;
		}

		GameObject roy = FactoryInstance.GetGOByName("ParticleSystemTest");
		auto& ps = roy.GetComponent<ParticleSystem>();

		static bool checkpoint1 = false;
		if (ps.systemTimer >= 1.0f && !checkpoint1)
		{
			checkpoint1 = true;
			json output = SliceEngine::JSONSerializer::SerializeGameObject(roy);
			SLICE_LOG("1 second mark");
		}

		static bool checkpoint2 = false;
		if (ps.systemTimer >= 2.0f && !checkpoint2)
		{
			checkpoint2 = true;
			json output = SliceEngine::JSONSerializer::SerializeGameObject(roy);
			SLICE_LOG("2 second mark");
			SLICE_LOG("Test 2 Ended.");
			testEnded = true;
		}
		
	}
#pragma endregion
}