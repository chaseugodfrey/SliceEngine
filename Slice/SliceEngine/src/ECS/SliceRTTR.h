/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			SliceRTTR.cpp
 author:		Gideon Francis
 email:			g.francis@digipen.edu
 brief:			RTTR

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef SLICE_RTTR_H
#define SLICE_RTTR_H

#include "../Graphics/WorldSpaceGraphicsSystem.h"
#include "../Systems/TransformSystem.h"
#include "../Systems/SoundSystem.h"
#include "Resource/ResourceManager.h"
#include "Resource/Resource.h"
#include "Resource/Model.h"
#include "Resource/Material.h"
#include "Animator/FSMSystem.h"
#include <rttr/registration.h>
namespace SliceEngine
{
	template<typename T, std::size_t N>
	void register_std_array(const std::string& name)
	{
		using Arr = std::array<T, N>;

		rttr::registration::class_<Arr>(name)
			.constructor<>()
			.method("size", &Arr::size)
			.method("at", static_cast<T & (Arr::*)(std::size_t)>(&Arr::at))
			.method("fill", &Arr::fill)
			.method("front", static_cast<T & (Arr::*)()>(&Arr::front))
			.method("back", static_cast<T & (Arr::*)()>(&Arr::back));
	}

	RTTR_REGISTRATION
	{
	rttr::registration::class_<glm::vec3>("vec3")
		.constructor<>()
		.property("x", &glm::vec3::x)
		.property("y", &glm::vec3::y)
		.property("z", &glm::vec3::z);

	rttr::registration::class_<glm::vec4>("vec4")
		.constructor<>()
		.property("x", &glm::vec4::x)
		.property("y", &glm::vec4::y)
		.property("z", &glm::vec4::z)
		.property("w", &glm::vec4::w);

	rttr::registration::class_<glm::quat>("quat")
		.constructor<>()
		.property("w", &glm::quat::w)
		.property("x", &glm::quat::x)
		.property("y", &glm::quat::y)
		.property("z", &glm::quat::z);

	rttr::registration::class_<std::vector<uint32_t>>("VectorUInt32")
		.constructor<>()
		.method("size", &std::vector<uint32_t>::size)
		.method("at", static_cast<uint32_t & (std::vector<uint32_t>::*)(size_t)>(&std::vector<uint32_t>::at))
		.method("push_back", static_cast<void (std::vector<uint32_t>::*)(const uint32_t&)>(&std::vector<uint32_t>::push_back))
		.method("push_back", static_cast<void (std::vector<uint32_t>::*)(uint32_t&&)>(&std::vector<uint32_t>::push_back));


	register_std_array<uint32_t, 4>("Array4UInt32");
	register_std_array<Entity, 4>("Array4Entity");

	rttr::registration::class_<glm::vec2>("glm::vec2")
		.constructor<>()(rttr::policy::ctor::as_object)
		.property("x", &glm::vec2::x)
		.property("y", &glm::vec2::y);

	rttr::registration::class_<glm::vec3>("glm::vec3")
		.constructor<>()(rttr::policy::ctor::as_object)
		.property("x", &glm::vec3::x)
		.property("y", &glm::vec3::y)
		.property("z", &glm::vec3::z);

	rttr::registration::class_<std::vector<glm::vec3>>("std::vector<glm::vec3>");
	rttr::registration::class_ <std::vector<std::string>>("std::vector<std::string>");
	rttr::registration::class_<std::vector<float>>("std::vector<float>");
	rttr::registration::class_<std::vector<int>>("std::vector<int>");

	rttr::registration::class_<std::string>("std::string")
		// Constructors
		.constructor<>()
		.constructor<const char*>()
		.constructor<const std::string&>()

		// Methods
		.method("size", &std::string::size)
		.method("length", &std::string::length)
		.method("clear", &std::string::clear)
		.method("empty", &std::string::empty)
		.method("c_str", &std::string::c_str);

	rttr::registration::class_<std::unordered_map<std::string, rttr::variant>>("StringVariantMap");

	rttr::registration::class_<Handle<SliceEngineTypes::Texture>>("Texture Handle")
		.constructor<>()
		.property("GUID", &Handle<SliceEngineTypes::Texture>::mGUID);

	rttr::registration::class_<Handle<SliceEngineTypes::Material>>("Material Handle")
		.constructor<>()
		.property("GUID", &Handle<SliceEngineTypes::Material>::mGUID);

	rttr::registration::class_<Handle<SliceEngineTypes::Model>>("Model Handle")
		.constructor<>()
		.property("GUID", &Handle<SliceEngineTypes::Model>::mGUID);

	rttr::registration::class_<Handle<SliceEngineTypes::StateMachine>>("stateMachine Handle")
		.constructor<>()
		.property("GUID", &Handle<SliceEngineTypes::StateMachine>::mGUID);

	rttr::registration::class_<FSMSystem>("stateMachine")
		.constructor<>()
		.property("EFSM", &FSMSystem::EFSM);

	rttr::registration::class_<Script>(typeid(Script).name())
		.property("scriptName", &Script::scriptName)
		.property("scriptableFieldMap", &Script::scriptableFieldMap);

	rttr::registration::class_<Transform>(typeid(Transform).name())
		.constructor<>()
		.property("position", &Transform::position)
		.property("rotation", &Transform::rotation)
		.property("scale", &Transform::scale)
		.property("euler_hint", &Transform::eulerAnglesHint)
		.property("collisionMask", &Transform::collisionMask);

	rttr::registration::class_<SceneGraph>(typeid(SceneGraph).name())
		.constructor<>()
		.property("entity_id", &SceneGraph::entity_id)
		.property("neighbours", &SceneGraph::neighbours);

	rttr::registration::class_<SliceEntity>(typeid(SliceEntity).name())
		.constructor<>()
		.property("mTag", &SliceEntity::mTag)
		.property("mName", &SliceEntity::mName);
	rttr::registration::class_<RigidBody>(typeid(RigidBody).name())
		.constructor<>()
		.property("isKinematic", &RigidBody::isKinematic)
		.property("gravityFactor", &RigidBody::gravityFactor)
		.property("CollisionDetection", &RigidBody::CollisionDetection)
		.property("mass", &RigidBody::mass)
		.property("friction", &RigidBody::friction)
		.property("restituition", &RigidBody::restitution)
		.property("linearDamping", &RigidBody::linearDamping)
		.property("angularDamping", &RigidBody::angularDamping);

	//Collider Shapes
	rttr::registration::class_<ColliderShape::BoxData>("BoxData")
		.constructor<>()
		.property("scale", &ColliderShape::BoxData::scale);

	rttr::registration::class_<ColliderShape::SphereData>("SphereData")
		.constructor<>()
		.property("radius", &ColliderShape::SphereData::radius);

	rttr::registration::class_<ColliderShape::CapsuleData>("CapsuleData")
		.constructor<>()
		.property("radius", &ColliderShape::CapsuleData::radius)
		.property("height", &ColliderShape::CapsuleData::height);

	rttr::registration::class_<ColliderShape>(typeid(ColliderShape).name())
		.constructor<>()
		.property("layer", &ColliderShape::layer)
		.property("boxData", &ColliderShape::GetBoxData, &ColliderShape::SetBoxData)
		.property("sphereData", &ColliderShape::GetSphereData, &ColliderShape::SetSphereData)
		.property("capsuleData", &ColliderShape::GetCapsuleData, &ColliderShape::SetCapsuleData)
		.property("offSet", &ColliderShape::offSet)
		.property("isTrigger", &ColliderShape::isTrigger);

	rttr::registration::class_<Renderer>(typeid(Renderer).name())
		.constructor<>()
		.property("model", &Renderer::modelHandle)
		.property("material", &Renderer::materialHandle)
		.property("renderTag", &Renderer::renderTag)
		.property("skinned", &Renderer::skinned) // If i do this, i'll need to serialize bone info and animator component
		.property("meshOffset", &Renderer::meshOffset);

	rttr::registration::class_<AudioSource>(typeid(AudioSource).name())
		.constructor<>()
		.property("soundGUID", &AudioSource::soundGUID)
		.property("channel", &AudioSource::channel)
		.property("previewChannel", &AudioSource::previewChannel)
		.property("currentVolume", &AudioSource::currentVolume)
		.property("isMute", &AudioSource::isMute)
		.property("isLoop", &AudioSource::isLoop)
		.property("isPaused", &AudioSource::isPaused)
		.property("pitch", &AudioSource::pitch)
		.property("stereoPan", &AudioSource::stereoPan)
		.property("spatialBlend", &AudioSource::spatialBlend)
		.property("dopplerLevel", &AudioSource::dopplerLevel)
		.property("spread", &AudioSource::spread)
		.property("minDistance", &AudioSource::minDistance)
		.property("maxDistance", &AudioSource::maxDistance)
		.property("priority", &AudioSource::priority)
		.property("playOnAwake", &AudioSource::playOnAwake)
		.property("volumeRollOff", &AudioSource::volumeRollOff)
		.property("minInterval", &AudioSource::minInterval)
		.property("_playTrigger", &AudioSource::_playTrigger)
		.property("playPreview", &AudioSource::playPreview);
		
	rttr::registration::class_<Camera>(typeid(Camera).name())
		.constructor<>()
		.property("width", &Camera::width)
		.property("height", &Camera::height)
		.property("pov", &Camera::pov)
		.property("near", &Camera::near)
		.property("far", &Camera::far)
		.property("textureID", &Camera::textureID)
		.property("depthTex", &Camera::depthTex)
		.property("renderTag", &Camera::renderTag)
		.property("fogColor", &Camera::fogColor)
		.property("fogIntensity", &Camera::fogIntensity)
		.property("bloomFilterRadius", &Camera::bloomFilterRadius)
		.property("vignetteCenter", &Camera::vignetteCenter)
		.property("vignetteIntensity", &Camera::vignetteIntensity)
		.property("vignetteSmoothness", &Camera::vignetteSmoothness);
	rttr::registration::class_<Script>(typeid(Script).name())
		.constructor<>()
		.property("scriptName", &Script::scriptName);
	rttr::registration::enumeration<Light::LightType>("LightType")
		(
			rttr::value("Directional", Light::LightType::Light_Directional),
			rttr::value("Point", Light::LightType::Light_Point),
			rttr::value("Spot", Light::LightType::Light_Spot)
		);
	rttr::registration::class_<Light>(typeid(Light).name())
		.constructor<>()
		.property("type", &Light::type)
		.property("color", &Light::color)
		.property("intensity", &Light::intensity);
	rttr::registration::class_<GUID>("GUID")
		.constructor<>()
		.constructor<uint64_t>()
		.property_readonly("Value", &GUID::GetGUID);

	rttr::registration::class_<Particle>(typeid(Particle).name())
		.constructor<>()
		.property("active", &Particle::active)
		.property("age", &Particle::age)
		.property("position", &Particle::position)
		.property("rotation", &Particle::rotation)
		.property("scale", &Particle::scale)
		.property("velocity", &Particle::velocity)
		.property("colour", &Particle::colour);

	rttr::registration::class_<ParticleSystem>(typeid(ParticleSystem).name())
		.constructor<>()
		// System settings
		.property("duration", &ParticleSystem::duration)
		.property("emissionRate", &ParticleSystem::emissionRate)
		.property("coneAngle", &ParticleSystem::coneAngle)
		.property("axis", &ParticleSystem::axis)
		.property("isRepeating", &ParticleSystem::isRepeating)
		.property("isLocalSpace",&ParticleSystem::isLocalSpace)
		.property("hasRandomParticleLifetime", &ParticleSystem::hasRandomParticleLifetime)
		.property("lifetime", &ParticleSystem::lifetime)
		.property("minParticleLifetime", &ParticleSystem::minParticleLifetime)
		.property("maxParticleLifetime", &ParticleSystem::maxParticleLifetime)
		.property("hasRandomSpawnPos", &ParticleSystem::hasRandomSpawnPos)
		.property("minRandomSpawnPos", &ParticleSystem::minRandomSpawnPos)
		.property("maxRandomSpawnPos", &ParticleSystem::maxRandomSpawnPos)
		.property("hasRandomInitialRotation", &ParticleSystem::hasRandomInitialRotation)
		.property("rotation", &ParticleSystem::rotation)
		.property("minRandomRotation", &ParticleSystem::minRandomRotation)
		.property("maxRandomRotation", &ParticleSystem::maxRandomRotation)
		.property("hasRandomVelocity", &ParticleSystem::hasRandomVelocity)
		.property("velocity", &ParticleSystem::velocity)
		.property("minRandomVelocity", &ParticleSystem::minRandomVelocity)
		.property("maxRandomVelocity", &ParticleSystem::maxRandomVelocity)
		.property("fadeOverLifetime", &ParticleSystem::fadeOverLifetime)
		.property("hasRandomScale", &ParticleSystem::hasRandomScale)
		.property("scale", &ParticleSystem::scale)
		.property("minRandomScale", &ParticleSystem::minRandomScale)
		.property("maxRandomScale", &ParticleSystem::maxRandomScale)
		.property("hasRandomColour", &ParticleSystem::hasRandomColour)
		.property("colour", &ParticleSystem::colour)
		.property("minRandomColour", &ParticleSystem::minRandomColour)
		.property("maxRandomColour", &ParticleSystem::maxRandomColour)
		.property("hasGravity", &ParticleSystem::hasGravity)
		.property("gForce", &ParticleSystem::gForce)
		.property("hasCollision", &ParticleSystem::hasCollision)
		.property("destroyOnExpire", &ParticleSystem::destroyOnExpire)
		.property("maxParticles", &ParticleSystem::maxParticles)
		.property("oldestIndex", &ParticleSystem::oldestIndex)
		.property("particles", &ParticleSystem::particles)
		// Bursts
		.property("hasBursts", &ParticleSystem::hasBursts)
		.property("numBursts", &ParticleSystem::numBursts)
		.property("bursts", &ParticleSystem::bursts);

	rttr::registration::class_<ParticleSystem::Burst>(typeid(ParticleSystem::Burst).name())
		.constructor<>()
		.property("numParticles", &ParticleSystem::Burst::numParticles)
		.property("burstRepetitions", &ParticleSystem::Burst::burstRepetitions)
		.property("burstPeriod", &ParticleSystem::Burst::burstPeriod)
		.property("triggerTime", &ParticleSystem::Burst::triggerTime)
		.property("triggered", &ParticleSystem::Burst::triggered);

	rttr::registration::class_<Animator>(typeid(Animator).name())
		.constructor<>()
		.property("current_time", &Animator::current_time)
		.property("stateMachine", &Animator::stateMachine);

	rttr::registration::class_<Bone>(typeid(Bone).name())
		.constructor<>()
		.property("skeleton_root", &Bone::skeleton_root)
		.property("frame_idx", &Bone::frame_idx);

	}
}
#endif