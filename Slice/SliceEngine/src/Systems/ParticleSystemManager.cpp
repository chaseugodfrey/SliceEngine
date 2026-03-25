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
#include <Systems/LayerManager.h>
#include "../Graphics/RenderManager.h"
#include "../Graphics/CameraSystem.h"
#include "../Serializer/JSONSerializer.h"
#include "../src/Physics/PhysicsSystem.h"
#include "Systems/SceneSystem.h"
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
		auto sceneSystem = Core::GetInstance()->GetSceneSystem();
		auto& ps = reg.get<ParticleSystem>(entity);

		if (ps.resetPreview)
		{
			ResetSystem(ps, dt);
			InitializeSystem(ps);
			ps.resetPreview = false;
		}

		if (sceneSystem->mCurrentState == SceneState::PLAY_SCENE)
		{
			ps.playPreview = false;
			ps.pausePreview = false;
		}
		else if (sceneSystem->mCurrentState == SceneState::DEFAULT && !ps.playPreview)
		{
			return;
		}
		else if (ps.pausePreview)
		{
			UpdateSystem(ps, 0.0f);
			return;
		}

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
		UpdateSystem(ps, dt);
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
		ps.delayTimer = 0.0f;
		ps.systemTimer = 0.0f;
		ps.systemEnding = false;
		ps.expired = false;
		ps.particles.clear();
		ps.particles.resize(ps.maxParticles);		
		ps.oldestIndex = 0u;
		ps.awaitingIndex = 0u;

		float inner = glm::clamp(ps.innerShapeRadius, 0.0f, ps.shapeRadius);
		float outer = ps.shapeRadius;
		ps.inner3 = inner * inner * inner;
		ps.outer3 = outer * outer * outer;

		ps.colourLifetimeMap.clear();
		for (const auto& kv : ps.colourMapIntermediary)
		{
			ps.colourLifetimeMap[kv.first] = kv.second;
		}

		ps.sizeMap.clear();
		for (const auto& kv : ps.sizeMapIntermediary)
		{
			ps.sizeMap.insert_or_assign(kv.first, kv.second);
		}

		ps.velocityMap.clear();
		for (const auto& kv : ps.velocityMapIntermediary)
		{
			ps.velocityMap.insert_or_assign(kv.first, kv.second);
		}

		try 
		{
			ps.renderData.reserve(ps.maxParticles);
		}
		catch (const std::bad_alloc& e) 
		{
			SLICE_LOG_ERROR("Particle System allocation failed:");
			SLICE_LOG_ERROR(e.what());
		}
	}
	void ParticleSystemManager::UpdateSystem(ParticleSystem& ps, float dt)
	{
		// Initial Delay
		if (ps.initialDelay > 0.0f && ps.delayTimer <= ps.initialDelay)
		{
			ps.delayTimer += dt;
			return;
		}

		ValidateParticleSystem(ps);

		// particle billboard from camera
		auto possibleCam = Core::GetInstance()->GetSystem<CameraSystem>().mainCam;
		glm::mat3 camRot{1.f};
		if(possibleCam.has_value())
			camRot = glm::mat3(glm::inverse(Core::GetInstance()->GetRegistry().get<Camera>(possibleCam.value()).V));
		glm::quat billboardRot = glm::quat_cast(camRot);

		// If system exceeded duration, flag as ending, if repeating, reset timer to dt
		if (ps.systemTimer >= ps.duration)
		{
			if (ps.isRepeating)
			{
				ResetSystem(ps, dt);				
			}
			else if (!ps.playPreview)
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

			if (ps.gForce != 0.0f)
			{
				ApplyGravity(p, ps, dt);
			}

			if (ps.colourOverLifetime)
			{
				ApplyColourOverLifetime(p, ps, dt);
			}

			if (ps.orbitOverLifetime)
			{
				ApplyOrbitOverLifetime(p, ps, dt);
			}

			if (ps.hasCollision)
			{
				ApplyPhysics(p, ps, dt);
			}

			ApplyVeloctiy(p, ps, dt);
		}

		if ((!haveActiveParticle && ps.systemEnding) && !ps.playPreview)
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
				transformMatrix = glm::translate(transformMatrix, p.position + ps.parentTransform->GetWorldPosition());
			}
			else
			{
				transformMatrix = glm::translate(transformMatrix, p.position);
			}
			
			// combine with system rotation
			glm::quat systemRot = glm::quat(glm::radians(ps.rotation3DHint));
			if (ps.parentTransform)
				systemRot = ps.parentTransform->rotation * systemRot;

			// particle rotation
			glm::quat particleRot = ps.isRotation3D ? p.rotation3D : glm::angleAxis(p.rotation, glm::vec3(0, 0, 1));

			// combine rotations if face camera
			glm::quat baseRot;

			if (ps.isLocalSpace || ps.followTransformRotation)
			{
				// inherit parent/system transform
				baseRot = systemRot * particleRot;
			}
			else
			{
				// independent particle
				baseRot = particleRot;
			}

			// billboard override
			if (ps.renderMode == ParticleSystem::RenderMode::BILLBOARD)
			{
				baseRot = particleRot; // or ignore systemRot entirely
			}
			
			// Rotation over time
			if (ps.rotateOverLifetime)
			{
				glm::quat deltaQ = RotateOverLifetime(p, ps, dt);
				baseRot = glm::normalize(deltaQ * baseRot);

				if (ps.isRotation3D)
					p.rotation3D = baseRot;
				else
					p.rotation = glm::eulerAngles(baseRot).z;
			}

			glm::quat finalRot = ps.alwaysFaceCamera ? (billboardRot * baseRot) : baseRot;

			transformMatrix *= glm::mat4_cast(finalRot);
			
			glm::vec3 finalScale = ps.sizeOverLifetime ? p.scale * SizeOverLifetime(p,ps,dt) : p.scale;
			transformMatrix = glm::scale(transformMatrix, finalScale);

			prp.transform = transformMatrix;
			prp.colour = p.colour;

			prp.isMeshParticle = (ps.renderMode == ParticleSystem::RenderMode::MESH) ? true : false;
			if (prp.isMeshParticle)
			{
				prp.modelGUID = ps.modelHandle.getGUID();
				prp.materialGUID = ps.materialHandle.getGUID();
			}
			else 
			{
				prp.textureID = ps.textureGUID.GetGUID();
				prp.isIgnoreLights = ps.ignoreLights;
			}

			if (ps.glow)
			{
				if (ps.glowValueType == ParticleSystem::ValueType::TWO_CONSTANTS)
				{
					std::uniform_real_distribution<float> distR(
						std::min(ps.minGlowIntensity, ps.maxGlowIntensity),
						std::max(ps.minGlowIntensity, ps.maxGlowIntensity)
					);

					prp.glowIntensity = distR(gen);
				}
				else
				{
					prp.glowIntensity = ps.glowIntensity;
				}
			}

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
#pragma region Init Stuff
	void ParticleSystemManager::InitializeLifetime(Particle& p, ParticleSystem& ps)
	{
		p.age = 0.0f;
		if (ps.initialLifetimeType == ParticleSystem::TWO_CONSTANTS)
		{
			std::uniform_real_distribution<float> randAge(
				std::min(ps.minParticleLifetime, ps.maxParticleLifetime),
				std::max(ps.minParticleLifetime, ps.maxParticleLifetime)
			);
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
			std::uniform_real_distribution<float> distX(
				std::min(ps.minRandomSpawnPos.x, ps.maxRandomSpawnPos.x),
				std::max(ps.minRandomSpawnPos.x, ps.maxRandomSpawnPos.x)
			);

			std::uniform_real_distribution<float> distY(
				std::min(ps.minRandomSpawnPos.y, ps.maxRandomSpawnPos.y),
				std::max(ps.minRandomSpawnPos.y, ps.maxRandomSpawnPos.y)
			);

			std::uniform_real_distribution<float> distZ(
				std::min(ps.minRandomSpawnPos.z, ps.maxRandomSpawnPos.z),
				std::max(ps.minRandomSpawnPos.z, ps.maxRandomSpawnPos.z)
			);

			// Assign a random position within the range
			p.position = glm::vec3(distX(gen), distY(gen), distZ(gen));
		}
		else 
		{
			p.position = ps.spawnPos;
		}

		if (!ps.isLocalSpace && ps.parentTransform)
		{
			p.position += ps.parentTransform->GetWorldPosition();
		}

		glm::vec3 offset;
		switch (ps.shapeType)
		{		
		case ParticleSystem::ShapeType::SPHERE:
			offset = RandomPointInSphere(ps);
			break;
		case ParticleSystem::ShapeType::CONE:
			offset = RandomPointInCircle(ps);
			break;
		case ParticleSystem::ShapeType::CUBE:
			offset = RandomPointInCube(ps);
			break;
		case ParticleSystem::ShapeType::CIRCLE:
			offset = RandomPointInCircle(ps);
			break;
		case ParticleSystem::ShapeType::RECT:
			offset = RandomPointInRect(ps);
			break;
		default:
			offset = RandomPointInSphere(ps);
			break;
		}

		//if (!ps.followTransformRotation)
		//	return localPoint;
		//else if (ps.parentTransform)
		//	return ps.parentTransform->rotation * localPoint;

		if (ps.parentTransform)
		{
			offset = ps.parentTransform->rotation * offset;
		}

		p.position += offset;
	}
	void ParticleSystemManager::InitializeRotation(Particle& p, ParticleSystem& ps)
	{
		if (ps.initialRotationType == ParticleSystem::ValueType::TWO_CONSTANTS)
		{
			if (ps.isRotation3D)
			{
				std::uniform_real_distribution<float> distX(
					std::min(ps.minRotation3DHint.x, ps.maxRotation3DHint.x),
					std::max(ps.minRotation3DHint.x, ps.maxRotation3DHint.x)
				);
				std::uniform_real_distribution<float> distY(
					std::min(ps.minRotation3DHint.y, ps.maxRotation3DHint.y),
					std::max(ps.minRotation3DHint.y, ps.maxRotation3DHint.y)
				);
				std::uniform_real_distribution<float> distZ(
					std::min(ps.minRotation3DHint.z, ps.maxRotation3DHint.z),
					std::max(ps.minRotation3DHint.z, ps.maxRotation3DHint.z)
				);

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
					std::min(ps.minRandomRotation, ps.maxRandomRotation),
					std::max(ps.minRandomRotation, ps.maxRandomRotation)
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
			std::uniform_real_distribution<float> distX(
				std::min(ps.minRandomScale.x, ps.maxRandomScale.x),
				std::max(ps.minRandomScale.x, ps.maxRandomScale.x)
			);
			std::uniform_real_distribution<float> distY(
				std::min(ps.minRandomScale.y, ps.maxRandomScale.y),
				std::max(ps.minRandomScale.y, ps.maxRandomScale.y)
			);
			std::uniform_real_distribution<float> distZ(
				std::min(ps.minRandomScale.z, ps.maxRandomScale.z),
				std::max(ps.minRandomScale.z, ps.maxRandomScale.z)
			);

			p.scale = glm::vec3(distX(gen), distY(gen), distZ(gen));
		}
		else 
		{
			p.scale = ps.scale;
		}
	}
	void ParticleSystemManager::InitializeVelocity(Particle& p, ParticleSystem& ps)
	{
		float speed{};
		if (ps.speedValueType == ParticleSystem::ValueType::TWO_CONSTANTS)
		{
			std::uniform_real_distribution<float> dist(
				std::min(ps.minRandomSpeed, ps.maxRandomSpeed),
				std::max(ps.minRandomSpeed, ps.maxRandomSpeed)
			);

			speed = dist(gen);
		}
		else
		{
			speed = ps.speed;
		}

		glm::vec3 direction(0.0f);

		switch (ps.shapeType)
		{
		case ParticleSystem::ShapeType::SPHERE:
			direction = ComputeSphereInitialVelocity(ps,p);
			break;
		case ParticleSystem::ShapeType::CONE:
			direction = RandomDirectionInCone(ps);
			break;
		case ParticleSystem::ShapeType::CIRCLE:
			direction = ComputeCircleInitialVelocity(ps,p);
			break;
		case ParticleSystem::ShapeType::CUBE:
			direction = RandomDirectionInCube(ps);
			break;
		case ParticleSystem::ShapeType::RECT:
			direction = ComputeRectInitialVelocity(ps);
			break;
		default:
			direction = glm::vec3(0.0f, 1.0f, 0.0f); // fallback
			break;
		}

		p.velocity = glm::normalize(direction) * speed;
	}
	void ParticleSystemManager::InitializeColour(Particle& p, ParticleSystem& ps)
	{
		if (ps.colourValueType == ParticleSystem::ValueType::TWO_CONSTANTS)
		{
			std::uniform_real_distribution<float> distR(
				std::min(ps.minRandomColour.r, ps.maxRandomColour.r),
				std::max(ps.minRandomColour.r, ps.maxRandomColour.r)
			);

			std::uniform_real_distribution<float> distG(
				std::min(ps.minRandomColour.g, ps.maxRandomColour.g),
				std::max(ps.minRandomColour.g, ps.maxRandomColour.g)
			);

			std::uniform_real_distribution<float> distB(
				std::min(ps.minRandomColour.b, ps.maxRandomColour.b),
				std::max(ps.minRandomColour.b, ps.maxRandomColour.b)
			);

			std::uniform_real_distribution<float> distA(
				std::min(ps.minRandomColour.a, ps.maxRandomColour.a),
				std::max(ps.minRandomColour.a, ps.maxRandomColour.a)
			);

			p.colour = glm::vec4(distR(gen), distG(gen), distB(gen), distA(gen));
		}
		else
		{
			p.colour = ps.colour;
		}
	}
#pragma endregion

#pragma region Applying Stuff
	void ParticleSystemManager::ApplyVeloctiy(Particle& p, ParticleSystem& ps, float dt)
	{
		if (ps.velocityOverLifetime)
		{
			glm::vec3 velocityMul = VelocityOverLifetime(p, ps, dt);
			p.position += p.velocity * velocityMul * dt;
		}
		else
		{
			p.position += p.velocity * dt;
		}
	}
	void ParticleSystemManager::ApplyGravity(Particle& p, ParticleSystem& ps, float dt)
	{
		p.velocity += glm::vec3(0.0f, -(ps.gForce * dt), 0.0f);
	}
	void ParticleSystemManager::ApplyPhysics(Particle& p, ParticleSystem& ps, float dt)
	{
		if (glm::length(p.velocity) < 0.01f) return;

		// Predict movement
		glm::vec3 end = p.position + p.velocity * dt;
		glm::vec3 direction = end - p.position;
		glm::vec3 hitPos, normal;
		uint32_t hitID;

		auto& physicsSystem = Core::GetInstance()->GetSystem<PhysicsSystem>();

		auto core = SliceEngine::Core::GetInstance();
		auto layer_manager = core->GetLayerManager();
		if (physicsSystem.PSystemRayCast(p.position, direction, hitID, hitPos, normal, false, layer_manager->GetCollisionMask(layer_manager->GetLayerName(ps.particleLayer))))
		{
			glm::vec3 n = glm::normalize(normal);
			float vn = glm::dot(p.velocity, n);          // velocity along normal
			float upDot = glm::dot(n, glm::vec3(0, 1, 0)); // normal vs world up

			// Determine if collision actually happened
			bool collisionHappened = vn < 0.0f || glm::length(hitPos - p.position) < 0.001f;

			if (collisionHappened)
			{
				// Penetration prevention
				float penetrationDepth = glm::dot(hitPos - p.position, n);
				if (penetrationDepth > 0.0f)
					p.position = hitPos + n * 0.001f; // small offset to avoid sticking

				// Normal and tangent velocity
				glm::vec3 vNormal = vn * n;
				glm::vec3 vTangent = p.velocity - vNormal;

				// Bounciness
				if (ps.bounciness > 0.0f && vn < 0.0f)
				{
					float bounceImpact = -vn * ps.bounciness;
					if (bounceImpact > 0.01f)
						vNormal = glm::reflect(vNormal, n) * ps.bounceDampening;
					else
						vNormal = glm::vec3(0.0f); // small impact, cancel normal
				}
				else
				{
					// prevent penetration if no bounce
					vNormal = glm::vec3(0.0f);
				}

				// Friction (only for mostly upward surfaces)
				if (upDot > 0.7f && ps.friction > 0.0f)
					vTangent *= (1.0f - ps.friction / 2.0f);

				// Stickiness
				if (ps.stickiness > 0.0f)
					vTangent *= (1.0f - ps.stickiness);

				// Final velocity
				p.velocity = vNormal + vTangent;
			}
		}
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

	glm::vec3 ParticleSystemManager::SizeOverLifetime(Particle& p, ParticleSystem& ps, float dt)
	{
		float t = glm::clamp(p.normalizedAge(), 0.0f, 1.0f);
		glm::vec3 scaleMul{ 1.0f }; // default scale

		if (ps.sizeMap.empty())
			return scaleMul;

		// If t is before the first key
		if (t <= ps.sizeMap.begin()->first)
			return ps.sizeMap.begin()->second;

		// If t is after the last key
		if (t >= ps.sizeMap.rbegin()->first)
			return ps.sizeMap.rbegin()->second;

		// Find the two keys between which t lies
		auto it = ps.sizeMap.lower_bound(t); // first key >= t

		if (it == ps.sizeMap.end())
			return ps.sizeMap.rbegin()->second;

		auto itPrev = std::prev(it);

		float t0 = itPrev->first;
		float t1 = it->first;

		const glm::vec3& v0 = itPrev->second;
		const glm::vec3& v1 = it->second;

		float factor = (t1 > t0) ? (t - t0) / (t1 - t0) : 0.0f;

		if (ps.sizeSeparateAxis)
		{
			// Lerp per-axis
			scaleMul = glm::mix(v0, v1, factor);
		}
		else
		{
			// Uniform scale using Z component
			float s = glm::mix(v0.z, v1.z, factor);
			scaleMul = glm::vec3(s);
		}

		return scaleMul;
	}

	glm::quat ParticleSystemManager::RotateOverLifetime(Particle& p, ParticleSystem& ps, float dt)
	{	
		glm::quat deltaQ;
		if (ps.rotateSeparateAxis)
		{
			// Angular velocity per axis (radians/sec)
			glm::vec3 deltaAngleRad = glm::radians(ps.rotateVelocity * dt);
			deltaQ = glm::quat(deltaAngleRad);
		}
		else
		{
			// Uniform rotation using Z as scalar
			float angleRad = glm::radians(ps.rotateVelocity.z * dt);
			deltaQ = glm::angleAxis(angleRad, glm::vec3(0, 0, 1));
		}

		return deltaQ;
	}

	void ParticleSystemManager::ApplyColourOverLifetime(Particle& p, ParticleSystem& ps, float dt)
	{
		auto& map = ps.colourLifetimeMap;

		if (map.empty())
			return;		

		float t = glm::clamp(p.normalizedAge(), 0.0f, 1.0f);

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

	glm::vec3 ParticleSystemManager::VelocityOverLifetime(Particle& p, ParticleSystem& ps, float dt)
	{
		float t = glm::clamp(p.normalizedAge(), 0.0f, 1.0f);
		glm::vec3 velocityMul{ 1.0f }; // default scale

		if (ps.velocityMap.empty())
			return velocityMul;

		// If t is before the first key
		if (t <= ps.velocityMap.begin()->first)
			return ps.velocityMap.begin()->second;

		// If t is after the last key
		if (t >= ps.velocityMap.rbegin()->first)
			return ps.velocityMap.rbegin()->second;

		// Find the two keys between which t lies
		auto it = ps.velocityMap.lower_bound(t); // first key >= t

		if (it == ps.velocityMap.end())
			return ps.velocityMap.rbegin()->second;

		auto itPrev = std::prev(it);

		float t0 = itPrev->first;
		float t1 = it->first;

		const glm::vec3& v0 = itPrev->second;
		const glm::vec3& v1 = it->second;

		float factor = (t1 > t0) ? (t - t0) / (t1 - t0) : 0.0f;

		if (ps.velocitySeparateAxis)
		{
			// Lerp per-axis
			velocityMul = glm::mix(v0, v1, factor);
		}
		else
		{
			// Uniform scale using Z component
			float s = glm::mix(v0.z, v1.z, factor);
			velocityMul = glm::vec3(s);
		}

		return velocityMul;
	}

	void ParticleSystemManager::ApplyOrbitOverLifetime(Particle& p, ParticleSystem& ps, float dt)
	{
		if (!ps.parentTransform)
		{
			SLICE_LOG_ERROR("Particle System Unable to apply orbit over lifetime as parent transform not found!");
			return;
		}

		if (ps.startOrbitVelocity == glm::vec3(0.0f) && ps.endOrbitVelocity == glm::vec3(0.0f))
			return;

		float t = glm::clamp(p.normalizedAge(), 0.0f, 1.0f);
		glm::vec3 omega = glm::mix(ps.startOrbitVelocity, ps.endOrbitVelocity, t);

		glm::quat rotX = glm::angleAxis(omega.x * dt, glm::vec3(1, 0, 0));
		glm::quat rotY = glm::angleAxis(omega.y * dt, glm::vec3(0, 1, 0));
		glm::quat rotZ = glm::angleAxis(omega.z * dt, glm::vec3(0, 0, 1));

		glm::quat deltaQ = rotZ * rotY * rotX;

		glm::vec3 offset = p.position - ps.parentTransform->GetWorldPosition();
		offset = deltaQ * offset;
		p.position = ps.parentTransform->GetWorldPosition() + offset;
	}
#pragma endregion

#pragma region Helper Functions
	glm::vec3 ParticleSystemManager::ComputeSphereInitialVelocity(ParticleSystem& ps, Particle& p, float radialBias)
	{
		std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

		// Direction from center to particle
		glm::vec3 center = glm::vec3(0.0f);

		if (!ps.isLocalSpace && ps.parentTransform)
		{
			center = ps.parentTransform->GetWorldPosition();
		}

		glm::vec3 dir = p.position - center;

		if (glm::dot(dir, dir) < 1e-6f)
			dir = glm::vec3(0.0f, 1.0f, 0.0f);
		else
			dir = glm::normalize(dir);

		radialBias = glm::clamp(radialBias, 0.0f, 1.0f);

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

	glm::vec3 ParticleSystemManager::ComputeRectInitialVelocity(ParticleSystem& ps)
	{
		glm::vec3 forward(0.0f, 0.0f, 1.0f);

		if (ps.parentTransform)
		{
			forward = ps.parentTransform->rotation * forward;
		}

		return glm::normalize(forward);
	}

	glm::vec3 ParticleSystemManager::ComputeCircleInitialVelocity(ParticleSystem& ps, Particle& p)
	{
		glm::vec3 center = glm::vec3(0.0f);

		if (!ps.isLocalSpace && ps.parentTransform)
		{
			center = ps.parentTransform->GetWorldPosition();
		}

		glm::vec3 dir = p.position - center;

		if (glm::dot(dir, dir) < 1e-6f)
			dir = glm::vec3(0.0f, 1.0f, 0.0f);
		else
			dir = glm::normalize(dir);

		// Prevent normalize(0,0,0)
		if (glm::dot(dir, dir) < 1e-6f)
		{
			// Fallback direction
			dir = glm::vec3(0.0f, 1.0f, 0.0f);
		}
		else
		{
			dir = glm::normalize(dir);
		}

		return dir;
	}

	glm::vec3 ParticleSystemManager::RandomDirectionInCone(ParticleSystem& ps)
	{
		std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

		float maxAngle = glm::radians(ps.coneArc) * 0.5f;
		float cosMax = cos(maxAngle);

		float u = dist01(gen);
		float v = dist01(gen);

		float cosTheta = glm::mix(cosMax, 1.0f, u);
		float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
		float phi = 2.0f * glm::two_pi<float>() * v;

		glm::vec3 localDir(
			cos(phi) * sinTheta,
			sin(phi) * sinTheta,
			cosTheta
		);

		glm::vec3 axis(0.0f, 0.0f, 1.0f);
		if (ps.parentTransform && ps.followTransformRotation)
		{
			axis = ps.parentTransform->rotation * glm::vec3(0.0f, 0.0f, 1.0f);
		}

		glm::quat q = Utilities::FromToRotation(glm::vec3(0.0f, 0.0f, 1.0f), axis); // desired world axis
		return q * localDir;
	}

	glm::vec3 ParticleSystemManager::RandomDirectionInCircle(ParticleSystem& ps)
	{
		std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

		float angle = dist01(gen) * glm::two_pi<float>();

		// Point on unit circle
		glm::vec3 dir(
			cos(angle),
			sin(angle),
			0.0f
		);

		if (ps.parentTransform && ps.followTransformRotation)
		{
			return ps.parentTransform->rotation * dir;
		}

		return dir;
	}

	glm::vec3 ParticleSystemManager::ComputeRectDirection(ParticleSystem& ps)
	{
		glm::vec3 forward(0.0f, 0.0f, 1.0f);

		if (ps.parentTransform)
		{
			return ps.parentTransform->rotation * forward;
		}

		return forward;
	}

	glm::vec3 ParticleSystemManager::RandomDirectionInCube(ParticleSystem& ps)
	{
		std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

		glm::vec3 dir;

		do {
			dir = glm::vec3(
				dist01(gen) * 2.0f - 1.0f,
				dist01(gen) * 2.0f - 1.0f,
				dist01(gen) * 2.0f - 1.0f
			);
		} while (glm::dot(dir, dir) < 1e-6f);

		dir = glm::normalize(dir);

		glm::vec3 axis(0.0f, 0.0f, 1.0f);
		if (ps.parentTransform && ps.followTransformRotation)
		{
			axis = ps.parentTransform->rotation * glm::vec3(0.0f, 0.0f, 1.0f);
		}

		glm::quat q = Utilities::FromToRotation(glm::vec3(0.0f, 0.0f, 1.0f), axis);

		return q * dir;
	}


	glm::vec3 ParticleSystemManager::RandomPointInSphere(ParticleSystem& ps)
	{
		if (ps.shapeRadius <= 0.0f)
			return glm::vec3(0.0f);

		std::uniform_real_distribution<float> dist(0.0f, 1.0f);

		float u = dist(gen);
		float w = dist(gen);

		float arcRad = glm::radians(
			glm::clamp(ps.sphereArc, 0.0f, 180.0f)
		);

		std::uniform_real_distribution<float> phiDist(0.0f, arcRad);
		float phi = phiDist(gen);

		float theta = 2.0f * glm::pi<float>() * u;

		float sinPhi = sin(phi);

		glm::vec3 dir(
			cos(theta) * sinPhi,
			sin(theta) * sinPhi,
			cos(phi)
		);

		float r = cbrt(
			ps.inner3 +
			w * (ps.outer3 - ps.inner3)
		);

		glm::vec3 localPoint = dir * r;

		return localPoint;
	}

	glm::vec3 ParticleSystemManager::RandomPointInCircle(ParticleSystem& ps)
	{
		if (ps.shapeRadius <= 0.0f)
			return glm::vec3(0.0f);

		std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

		float inner = glm::clamp(ps.innerShapeRadius, 0.0f, ps.shapeRadius);
		float outer = ps.shapeRadius;

		float r = sqrt(
			inner * inner +
			dist01(gen) * (outer * outer - inner * inner)
		);

		float theta = 2.0f * glm::pi<float>() * dist01(gen);
				
		return glm::vec3(
			r * cos(theta),
			r * sin(theta),
			0.0f
		);
	}

	glm::vec3 ParticleSystemManager::RandomPointInCube(ParticleSystem& ps)
	{
		glm::vec3 halfExtents = ps.shapeScale * 0.5f;

		halfExtents.x = std::max(0.0f, halfExtents.x);
		halfExtents.y = std::max(0.0f, halfExtents.y);
		halfExtents.z = std::max(0.0f, halfExtents.z);

		std::uniform_real_distribution<float> distX(-halfExtents.x, halfExtents.x);
		std::uniform_real_distribution<float> distY(-halfExtents.y, halfExtents.y);
		std::uniform_real_distribution<float> distZ(-halfExtents.z, halfExtents.z);

		return glm::vec3(
			distX(gen),
			distY(gen),
			distZ(gen)
		);
	}

	glm::vec3 ParticleSystemManager::RandomPointInRect(ParticleSystem& ps)
	{
		glm::vec2 halfExtents = ps.rectScale * 0.5f;

		halfExtents.x = std::max(0.0f, halfExtents.x);
		halfExtents.y = std::max(0.0f, halfExtents.y);

		std::uniform_real_distribution<float> distX(-halfExtents.x, halfExtents.x);
		std::uniform_real_distribution<float> distY(-halfExtents.y, halfExtents.y);

		return glm::vec3(
			distX(gen),
			distY(gen),
			0.0f
		);
	}
#pragma endregion

#pragma region Tests
	void ParticleSystemManager::RunTests()
	{
		ParticleSystem ps{};

		SLICE_LOG("Creation Test Begin..");
		CreationTest(ps);
		SLICE_LOG("Creation Test Ended.");

		SLICE_LOG("Update Test Begin..");
		UpdateTest(ps, 0.016f);
		SLICE_LOG("Update Test Ended.");
	}

	void ParticleSystemManager::CreationTest(ParticleSystem& ps)
	{
		// Basic config
		ps.maxParticles = 100;
		ps.duration = 5.0f;
		ps.isRepeating = false;
		ps.emissionRate = 20.0f;
		ps.lifetime = 2.0f;
		ps.speed = 3.0f;
		ps.shapeType = ParticleSystem::ShapeType::SPHERE;
		ps.shapeRadius = 2.0f;
		ps.spawnPos = glm::vec3(0.0f);

		ps.scale = glm::vec3(1.0f);
		ps.rotation = 0.0f;
		ps.colour = glm::vec4(1.0f);

		ps.initialLifetimeType = ParticleSystem::ValueType::CONSTANT;
		ps.posValueType = ParticleSystem::ValueType::TWO_CONSTANTS;
		ps.scaleType = ParticleSystem::ValueType::CONSTANT;
		ps.speedValueType = ParticleSystem::ValueType::TWO_CONSTANTS;
		ps.colourValueType = ParticleSystem::ValueType::CONSTANT;

		// Size over lifetime test
		ps.sizeOverLifetime = true;
		ps.sizeMap.clear();
		ps.sizeMap[0.0f] = glm::vec3(1.0f);
		ps.sizeMap[1.0f] = glm::vec3(0.0f);

		// Colour over lifetime test
		ps.colourOverLifetime = true;
		ps.colourLifetimeMap.clear();
		ps.colourLifetimeMap[0.0f] = glm::vec4(1, 0, 0, 1);
		ps.colourLifetimeMap[1.0f] = glm::vec4(0, 0, 1, 0);

		// Velocity over lifetime
		ps.velocityOverLifetime = true;
		ps.velocityMap.clear();
		ps.velocityMap[0.0f] = glm::vec3(1.0f);
		ps.velocityMap[1.0f] = glm::vec3(0.5f);

		InitializeSystem(ps);

		if (ps.particles.size() != ps.maxParticles)
			SLICE_LOG_ERROR("CreationTest FAILED: Particle pool size mismatch.");

		if (ps.sizeMap.size() != 2)
			SLICE_LOG_ERROR("CreationTest FAILED: Size map not copied correctly.");

		if (ps.colourLifetimeMap.size() != 2)
			SLICE_LOG_ERROR("CreationTest FAILED: Colour map not copied correctly.");

		if (ps.velocityMap.size() != 2)
			SLICE_LOG_ERROR("CreationTest FAILED: Velocity map not copied correctly.");

		SLICE_LOG("CreationTest completed.");
	}

	void ParticleSystemManager::UpdateTest(ParticleSystem& ps, float fakeDt)
	{
		InitializeSystem(ps);

		unsigned int numFrames = 60;

		for (unsigned int i = 0; i < numFrames; ++i)
		{
			UpdateSystem(ps, fakeDt);
		}

		// Check some particles spawned
		bool foundActive = false;
		for (auto& p : ps.particles)
		{
			if (p.active)
			{
				foundActive = true;
				break;
			}
		}

		if (!foundActive)
			SLICE_LOG_ERROR("UpdateTest FAILED: No active particles after updates.");

		// Check render data
		if (ps.renderData.empty())
			SLICE_LOG_ERROR("UpdateTest FAILED: No render data generated.");

		SLICE_LOG("UpdateTest completed.");
	}

#pragma endregion
}