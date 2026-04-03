/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Engine.cpp
 author:
 email:
 brief:			Main Engine

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "Engine.h"
#include "ECS/ECSTypes.h"
#include "Physics/PhysicsSystem.h"
#include "GLFWWindowManager.h"
#include "Core/Core.h"
#include "Input/InputSystem.h"
#include "../src/Audio/AudioManager.h"
#include "Systems/TransformSystem.h"
#include <crtdbg.h>
//#include "Graphics/ResourceManager.h"
#include "Resource/ResourceManager.h"

#include "Graphics/CameraSystem.h"
#include "Graphics/RenderManager.h"
#include "Graphics/LightingSystem.h"
#include "Graphics/CanvasSystem.h"
#include "Graphics/SpriteAnimationSystem.h"
#include "Graphics/UI_Interactible.h"
#include "ECS/BaseSystem.h"
#include "ECS/SliceRTTR.h"
#include "Systems/FramerateManager.h"
#include "Serializer/JSONSerializer.h"
#include "Serializer/CSVSerializer.h"
#include "Graphics/TransformHelper.h"
#include "Scripting/ScriptSystem.h"
#include "Systems/SceneSystem.h"
#include "Configuration/ProjectSettingsManager.h"
#include "Networking/NetworkSystem.h"
#include "Systems/ParticleSystemManager.h"
#include "Systems/PrefabSystem.h"
#include "Input/ActionMapping.h"
#include "Animator/AnimatorSystem.h"
#include "Animator/BoneSystem.h"
#include "Systems/LayerManager.h"
#include "Configuration/AudioSettings.cpp"

#pragma region RTTR REGISTRATION STUFF
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
		.method("at", static_cast<uint32_t& (std::vector<uint32_t>::*)(size_t)>(&std::vector<uint32_t>::at))
		.method("push_back", static_cast<void (std::vector<uint32_t>::*)(const uint32_t&)>(&std::vector<uint32_t>::push_back))
		.method("push_back", static_cast<void (std::vector<uint32_t>::*)(uint32_t&&)>(&std::vector<uint32_t>::push_back));


	register_std_array<Entity, 4>("Array4Entity");
	register_std_array<uint32_t, 4>("Array4UInt32");
	//register_std_array<glm::vec4, Button::Total_States>("ArrayBtnStates");
	//register_std_array<float, Button::Total_States>("ArrayTest");

	rttr::registration::class_<glm::vec2>("glm::vec2")
		.constructor<>()(rttr::policy::ctor::as_object)
		.property("x", &glm::vec2::x)
		.property("y", &glm::vec2::y);

	rttr::registration::class_<glm::vec3>("glm::vec3")
		.constructor<>()(rttr::policy::ctor::as_object)
		.property("x", &glm::vec3::x)
		.property("y", &glm::vec3::y)
		.property("z", &glm::vec3::z);

#pragma warning(push)
#pragma warning(disable: 4189)
#pragma warning(disable: 26444)
	rttr::registration::class_<std::vector<glm::vec3>>("std::vector<glm::vec3>");
	rttr::registration::class_<std::vector<std::string>>("std::vector<std::string>");
	rttr::registration::class_<std::vector<float>>("std::vector<float>");
	rttr::registration::class_<std::vector<int>>("std::vector<int>");
	rttr::registration::class_<GameObject>("SliceEngine::GameObject");
	rttr::registration::class_<std::vector<GameObject>>("std::vector<SliceEngine::GameObject>");
	rttr::registration::class_<std::vector<PrefabVar>>("std::vector<SliceEngine::PrefabVar>");
	rttr::registration::class_<PrefabVar>("SliceEngine::PrefabVar");
#pragma warning(pop)
#pragma warning(pop)

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

	rttr::registration::class_<Handle<SliceEngineTypes::Skeleton>>("Skeleton Handle")
		.constructor<>()
		.property("GUID", &Handle<SliceEngineTypes::Skeleton>::mGUID);

	rttr::registration::class_<Handle<SliceEngineTypes::AnimationPackage>>("AnimPkg Handle")
		.constructor<>()
		.property("GUID", &Handle<SliceEngineTypes::AnimationPackage>::mGUID);

	rttr::registration::class_<Handle<SliceEngineTypes::StateMachine>>("stateMachine Handle")
		.constructor<>()
		.property("GUID", &Handle<SliceEngineTypes::StateMachine>::mGUID);
	rttr::registration::class_<Handle<SliceEngineTypes::Prefab>>("Prefab Handle")
		.constructor<>()
		.property("GUID", &Handle<SliceEngineTypes::Prefab>::mGUID);
	rttr::registration::class_<Script>(typeid(Script).name())
		.property("scriptName", &Script::scriptName)
		.property("scriptableFieldMap", &Script::scriptableFieldMap);

	rttr::registration::class_<Transform>(typeid(Transform).name())
		.constructor<>()
		.property("position", &Transform::position)
		.property("rotation", &Transform::rotation)
		.property("scale", &Transform::scale)
		.property("euler_hint", &Transform::eulerAnglesHint);

	rttr::registration::class_<SceneGraph>(typeid(SceneGraph).name())
		.constructor<>()
		.property("entity_id", &SceneGraph::entity_id)
		.property("neighbours", &SceneGraph::neighbours);

	rttr::registration::class_<SliceEntity>(typeid(SliceEntity).name())
		.constructor<>()
		.property("mActive", &SliceEntity::mActive)
		.property("mTag", &SliceEntity::mTag)
		.property("mName", &SliceEntity::mName)
		.property("mLayer", &SliceEntity::mLayer);

	rttr::registration::class_<InactiveEntity>(typeid(InactiveEntity).name())
		.constructor<>()
		.property("mTest", &InactiveEntity::mTest);

	rttr::registration::class_<RigidBody::FreezeOptions>("FreezeOptions")
		.constructor<>()
		.property("freezeX", &RigidBody::FreezeOptions::freezeX)
		.property("freezeY", &RigidBody::FreezeOptions::freezeY)
		.property("freezeZ", &RigidBody::FreezeOptions::freezeZ);


	rttr::registration::class_<RigidBody>(typeid(RigidBody).name())
		.constructor<>()
		.property("isKinematic", &RigidBody::isKinematic)
		.property("gravityFactor", &RigidBody::gravityFactor)
		.property("CollisionDetection", &RigidBody::CollisionDetection)
		.property("mass", &RigidBody::mass)
		.property("friction", &RigidBody::friction)
		.property("restituition", &RigidBody::restitution)
		.property("linearDamping", &RigidBody::linearDamping)
		.property("angularDamping", &RigidBody::angularDamping)
		.property("freezePosition", &RigidBody::freezePosition)
		.property("freezeRotation", &RigidBody::freezeRotation);

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

	rttr::registration::class_<ColliderShape::MeshData>("MeshData")
		.constructor<>()
		.property("UwU", &ColliderShape::MeshData::temp);

	rttr::registration::class_<ColliderShape::CylinderData>("CylinderData")
		.constructor<>()
		.property("radius", &ColliderShape::CapsuleData::radius)
		.property("height", &ColliderShape::CapsuleData::height);

	rttr::registration::class_<ColliderShape>(typeid(ColliderShape).name())
		.constructor<>()
		.property("boxData", &ColliderShape::GetBoxData, &ColliderShape::SetBoxData)
		.property("sphereData", &ColliderShape::GetSphereData, &ColliderShape::SetSphereData)
		.property("capsuleData", &ColliderShape::GetCapsuleData, &ColliderShape::SetCapsuleData)
		.property("meshData", &ColliderShape::GetMeshData, &ColliderShape::SetMeshData)
		.property("cylinderData", &ColliderShape::GetCylinderData, &ColliderShape::SetCylinderData)
		.property("offSet", &ColliderShape::offSet)
		.property("isTrigger", &ColliderShape::isTrigger)
		.property("componentEnabled", &ColliderShape::componentEnabled);

	rttr::registration::class_<Renderer>(typeid(Renderer).name())
		.constructor<>()
		.property("model", &Renderer::modelHandle)
		.property("material", &Renderer::materialHandle)
		.property("renderTag", &Renderer::renderTag)
		.property("skinned", &Renderer::skinned) // If i do this, i'll need to serialize bone info and animator component
		.property("castsShadow", &Renderer::castShadow)
		.property("meshOffset", &Renderer::meshOffset)
		.property("componentEnabled", &Renderer::componentEnabled);

	rttr::registration::class_<AudioSource>(typeid(AudioSource).name())
		.constructor<>()
		.property("soundGUID", &AudioSource::soundGUID)
		.property("currentVolume", &AudioSource::currentVolume)
		.property("isMute", &AudioSource::isMute)
		.property("isLoop", &AudioSource::isLoop)
		.property("isPaused", &AudioSource::isPaused)
		.property("pitch", &AudioSource::pitch)
		.property("stereoPan", &AudioSource::stereoPan)
		.property("spatialBlend", &AudioSource::spatialBlend)
		.property("dopplerLevel", &AudioSource::dopplerLevel)
		.property("category", &AudioSource::category)
		.property("spread", &AudioSource::spread)
		.property("minDistance", &AudioSource::minDistance)
		.property("maxDistance", &AudioSource::maxDistance)
		.property("priority", &AudioSource::priority)
		.property("playOnAwake", &AudioSource::playOnAwake)
		.property("volumeRollOff", &AudioSource::volumeRollOff)
		.property("playPreview", &AudioSource::playPreview)
		.property("directOcclusion", &AudioSource::directOcclusion)
		.property("reverbOcclusion", &AudioSource::reverbOcclusion)
		.property("componentEnabled", &AudioSource::componentEnabled);

	rttr::registration::class_<AudioListener>(typeid(AudioListener).name())
		.constructor<>()
		.property("listenerPos", &AudioListener::listenerPos)
		.property("componentEnabled", &AudioListener::componentEnabled);


	rttr::registration::class_<Camera>(typeid(Camera).name())
		.constructor<>()
		.property("width", &Camera::width)
		.property("height", &Camera::height)
		.property("pov", &Camera::pov)
		.property("near", &Camera::near)
		.property("far", &Camera::far)
		.property("textureID", &Camera::textureID)
		.property("depthTex", &Camera::depthTex)
		.property("debugRenderTag", &Camera::debugRenderToggles)
		.property("postRenderTag", &Camera::postRenderToggles)
		.property("luminanceLearningRate", &Camera::luminanceLearningRate)
		.property("fogColor", &Camera::fogColor)
		.property("fogIntensity", &Camera::fogIntensity)
		.property("bloomStrength", &Camera::bloomStrength)
		.property("bloomLimit", &Camera::bloomLimit)
		.property("bloomFilterRadius", &Camera::bloomFilterRadius)
		.property("bloomExposure", &Camera::exposure)
		.property("gamma", &Camera::gamma)
		.property("whiteBalance", &Camera::whiteBalance)
		.property("minLuminance", &Camera::minLuminance)
		.property("maxLuminance", &Camera::maxLuminance)
		.property("godRayStrength", &Camera::godRayStrength)
		.property("godRayFilterRadius", &Camera::godRayFilterRadius)
		.property("vignetteCenter", &Camera::vignetteCenter)
		.property("vignetteIntensity", &Camera::vignetteIntensity)
		.property("vignetteSmoothness", &Camera::vignetteSmoothness)
		.property("impactPosition", &Camera::impactPos)
		.property("impactColor", &Camera::impactColor)
		.property("impactColor2", &Camera::impactColor2)
		.property("impactAngle", &Camera::impactAngle)
		.property("impactSmoothness", &Camera::impactSmooth)
		.property("impactEpilepsy", &Camera::impactEpilepsy)
		.property("impactNoise1", &Camera::impactNoise1)
		.property("impactNoise2", &Camera::impactNoise2)
		.property("impactBlend", &Camera::impactBlend)
		.property("cloudsHeight", &Camera::cloudsHeight)
		.property("cloudsAmplitute", &Camera::cloudsAmplitude)
		.property("cloudsIntensity", &Camera::cloudsIntensity)
		.property("cloudsSmoothness", &Camera::cloudsSmoothness)
		.property("cloudsCutoff", &Camera::cloudsCutoff)
		.property("cloudsColor", &Camera::cloudsColor)
		.property("cloudsSecondOffset", &Camera::cloudsSecondCloudOffset)
		.property("cloudsSecondAmplitute", &Camera::cloudsSecondCloudAmplitude)
		.property("cloudsSecondIntensity", &Camera::cloudsSecondCloudIntensity)
		.property("cloudsSecondSmoothness", &Camera::cloudsSecondCloudSmoothness)
		.property("translucentSelectCutoff", &Camera::translucentSelectCutoff)
		.property("cloudsSecondColor", &Camera::cloudsSecondColor)
		.property("isMainCamera", &Camera::isMainCamera)
		.property("componentEnabled", &Camera::componentEnabled);

	rttr::registration::class_<Script>(typeid(Script).name())
		.constructor<>()
		.property("scriptName", &Script::scriptName);

	rttr::registration::enumeration<Light::LightType>("LightType")
		(
			rttr::value("Directional", Light::LightType::Light_Directional),
			rttr::value("Point", Light::LightType::Light_Point),
			rttr::value("Spot", Light::LightType::Light_Spot)
		);
	rttr::registration::enumeration<RectTransform::HoriPivot>("HoriPivot")
		(
			rttr::value("Left", RectTransform::HoriPivot::LEFT),
			rttr::value("Center", RectTransform::HoriPivot::CENTER),
			rttr::value("Right", RectTransform::HoriPivot::RIGHT),
			rttr::value("Stretch", RectTransform::HoriPivot::STRETCH_H)
			);
	rttr::registration::enumeration<RectTransform::VertPivot>("VertPivot")
		(
			rttr::value("Top", RectTransform::VertPivot::TOP),
			rttr::value("Middle", RectTransform::VertPivot::MIDDLE),
			rttr::value("Bottom", RectTransform::VertPivot::BOTTOM),
			rttr::value("Stretch", RectTransform::VertPivot::STRETCH_V)
			);
	rttr::registration::enumeration<Canvas::Type>("CanvasType")
		(
			rttr::value("Overlay", Canvas::Type::OVERLAY),
			rttr::value("World Space", Canvas::Type::WORLD)
			);
	rttr::registration::enumeration<FontRenderer::Alignment>("FontAlignment")
		(
			rttr::value("Left", FontRenderer::LEFT),
			rttr::value("Center", FontRenderer::CENTER),
			rttr::value("Right", FontRenderer::RIGHT)
			);
	rttr::registration::enumeration<Button::Transition>("ButtonTransition")
		(
			rttr::value("Color", Button::Transition::Color),
			rttr::value("Sprite", Button::Transition::Sprite)
			);
	rttr::registration::enumeration<Slider::Axis>("SliderAxis")
		(
			rttr::value("X_Axis", Slider::Axis::X_Axis),
			rttr::value("Y_Axis", Slider::Axis::Y_Axis)
			);
	rttr::registration::enumeration<Slider::Direction>("SliderDirection")
		(
			rttr::value("Positive", Slider::Direction::Positive),
			rttr::value("Negative", Slider::Direction::Negative)
			);
	rttr::registration::enumeration<AudioSource::VolumeRollOff>("VolumeRollOff")
		(
			rttr::value("Logarithmic", AudioSource::VolumeRollOff::Logarithmic),
			rttr::value("Linear", AudioSource::VolumeRollOff::Linear)
		);
	rttr::registration::enumeration<AudioSource::Category>("Category")
		(
			rttr::value("SFX", AudioSource::Category::SFX),
			rttr::value("BGM", AudioSource::Category::BGM),
			rttr::value("UI", AudioSource::Category::UI)
		);
	rttr::registration::class_<Light>(typeid(Light).name())
		.constructor<>()
		.property("type", &Light::type)
		.property("color", &Light::color)
		.property("intensity", &Light::intensity)
		.property("angle", &Light::angle)
		.property("componentEnabled", &Light::componentEnabled)
		.property("castsShadow", &Light::castsShadow);

	rttr::registration::class_<GUID>("GUID")
		.constructor<>()
		.constructor<uint64_t>()
		.property_readonly("Value", &GUID::GetGUID);

	rttr::registration::class_<SliceEngineTypes::AnimationKeyFrame>("Animation Key Frames")
		.constructor<>()
		.property("scriptName", &SliceEngineTypes::AnimationKeyFrame::scriptName)
		.property("scriptFunc", &SliceEngineTypes::AnimationKeyFrame::scriptFunc)
		.property("animIdx", &SliceEngineTypes::AnimationKeyFrame::animIdx)
		.property("frameNumber", &SliceEngineTypes::AnimationKeyFrame::frameNumber);

	rttr::registration::class_<std::pair<float, glm::vec4>>("PairFloatVec4")
		.constructor<>()
		.property("first", &std::pair<float, glm::vec4>::first)
		.property("second", &std::pair<float, glm::vec4>::second);

	rttr::registration::class_<std::vector<std::pair<float, glm::vec4>>>("VectorPairFloatVec4");

	rttr::registration::class_<std::vector<SliceEngineTypes::AnimationKeyFrame>>("std::vector<SliceEngineTypes::AnimationKeyFrame");

	rttr::registration::class_<std::map<float, glm::vec3>>("map_float_vec3")
		.constructor<>()
		.method("size", [](std::map<float, glm::vec3>& m) { return m.size(); })
		.method("insert_or_assign", static_cast<std::pair<std::map<float, glm::vec3>::iterator, bool>
			(std::map<float, glm::vec3>::*)(const float&, const glm::vec3&)>(&std::map<float, glm::vec3>::insert_or_assign))
		.method("clear", &std::map<float, glm::vec3>::clear);

	rttr::registration::class_<std::map<float, glm::vec4>>("map_float_vec4")
		.constructor<>()
		.method("size", [](std::map<float, glm::vec4>& m) { return m.size(); })
		.method("insert_or_assign", static_cast<std::pair<std::map<float, glm::vec4>::iterator, bool>
			(std::map<float, glm::vec4>::*)(const float&, const glm::vec4&)>(&std::map<float, glm::vec4>::insert_or_assign))
		.method("clear", &std::map<float, glm::vec4>::clear);

	rttr::registration::enumeration<ParticleSystem::ShapeType>(typeid(ParticleSystem::ShapeType).name())
		(
			rttr::value("SPHERE", ParticleSystem::ShapeType::SPHERE),
			rttr::value("CONE", ParticleSystem::ShapeType::CONE),
			rttr::value("CUBE", ParticleSystem::ShapeType::CUBE),
			rttr::value("CIRCLE", ParticleSystem::ShapeType::CIRCLE),
			rttr::value("RECT", ParticleSystem::ShapeType::RECT)
			);

	rttr::registration::class_<Particle>(typeid(Particle).name());

	rttr::registration::enumeration<ParticleSystem::ValueType>("ValueType")
		(
			rttr::value("CONSTANT", ParticleSystem::ValueType::CONSTANT),
			rttr::value("TWO_CONSTANTS", ParticleSystem::ValueType::TWO_CONSTANTS)
			);

	rttr::registration::enumeration<ParticleSystem::RenderMode>("RenderMode")
		(
			rttr::value("BILLBOARD", ParticleSystem::RenderMode::BILLBOARD),
			rttr::value("MESH", ParticleSystem::RenderMode::MESH)
			);

	rttr::registration::class_<ParticleSystem>(typeid(ParticleSystem).name())
		.constructor<>()
		.property("initialDelay", &ParticleSystem::initialDelay)
		.property("duration", &ParticleSystem::duration)
		.property("isRepeating", &ParticleSystem::isRepeating)
		.property("isLocalSpace", &ParticleSystem::isLocalSpace)
		.property("followTransformRotation", &ParticleSystem::followTransformRotation)

		.property("destroyOnExpire", &ParticleSystem::destroyOnExpire)
		.property("maxParticles", &ParticleSystem::maxParticles)

		.property("gForce", &ParticleSystem::gForce)
		.property("hasCollision", &ParticleSystem::hasCollision)
		.property("friction", &ParticleSystem::friction)
		.property("bounciness", &ParticleSystem::bounciness)
		.property("bounceDampening", &ParticleSystem::bounceDampening)
		.property("stickiness", &ParticleSystem::stickiness)

		.property("emissionRate", &ParticleSystem::emissionRate)

		.property("numBursts", &ParticleSystem::numBursts)
		.property("bursts", &ParticleSystem::bursts)

		.property("shapeType", &ParticleSystem::shapeType)

		.property("coneArc", &ParticleSystem::coneArc)

		.property("sphereArc", &ParticleSystem::sphereArc)

		.property("rectScale", &ParticleSystem::rectScale)

		.property("shapeRadius", &ParticleSystem::shapeRadius)
		.property("shapeScale", &ParticleSystem::shapeScale)
		.property("innerShapeRadius", &ParticleSystem::innerShapeRadius)

		.property("axis", &ParticleSystem::axis)

		.property("scaleType", &ParticleSystem::scaleType)
		.property("scale", &ParticleSystem::scale)
		.property("minRandomScale", &ParticleSystem::minRandomScale)
		.property("maxRandomScale", &ParticleSystem::maxRandomScale)

		.property("initialLifetimeType", &ParticleSystem::initialLifetimeType)
		.property("lifetime", &ParticleSystem::lifetime)
		.property("minParticleLifetime", &ParticleSystem::minParticleLifetime)
		.property("maxParticleLifetime", &ParticleSystem::maxParticleLifetime)

		.property("initialRotationType", &ParticleSystem::initialRotationType)
		.property("rotation", &ParticleSystem::rotation)
		.property("minRandomRotation", &ParticleSystem::minRandomRotation)
		.property("maxRandomRotation", &ParticleSystem::maxRandomRotation)

		.property("isRotation3D", &ParticleSystem::isRotation3D)
		.property("rotation3D", &ParticleSystem::rotation3DHint)
		.property("minRandomRotation3D", &ParticleSystem::minRotation3DHint)
		.property("maxRandomRotation3D", &ParticleSystem::maxRotation3DHint)

		.property("spawnPosValueType", &ParticleSystem::posValueType)
		.property("spawnPos", &ParticleSystem::spawnPos)
		.property("minRandomSpawnPos", &ParticleSystem::minRandomSpawnPos)
		.property("maxRandomSpawnPos", &ParticleSystem::maxRandomSpawnPos)

		.property("colorValueType", &ParticleSystem::colourValueType)
		.property("colour", &ParticleSystem::colour)
		.property("minRandomColour", &ParticleSystem::minRandomColour)
		.property("maxRandomColour", &ParticleSystem::maxRandomColour)

		.property("speedValueType", &ParticleSystem::speedValueType)
		.property("speed", &ParticleSystem::speed)
		.property("minRandomSpeed", &ParticleSystem::minRandomSpeed)
		.property("maxRandomSpeed", &ParticleSystem::maxRandomSpeed)

		.property("sizeOverLifetime", &ParticleSystem::sizeOverLifetime)
		.property("sizeSeparateAxis", &ParticleSystem::sizeSeparateAxis)
		.property("sizeMap", &ParticleSystem::sizeMap)
		.property("sizeMapIntermediary", &ParticleSystem::sizeMapIntermediary)

		.property("rotateOverLifetime", &ParticleSystem::rotateOverLifetime)
		.property("rotateSeparateAxis", &ParticleSystem::rotateSeparateAxis)
		.property("rotateVelocity", &ParticleSystem::rotateVelocity)

		.property("colourOverLifetime", &ParticleSystem::colourOverLifetime)
		.property("colourLifetimeMap", &ParticleSystem::colourLifetimeMap)
		.property("colourMapIntermediary", &ParticleSystem::colourMapIntermediary)

		.property("velocityOverLifetime", &ParticleSystem::velocityOverLifetime)
		.property("velocitySeparateAxis", &ParticleSystem::velocitySeparateAxis)
		.property("velocityMap", &ParticleSystem::velocityMap)
		.property("velocityMapIntermediary", &ParticleSystem::velocityMapIntermediary)

		.property("orbitOverLifetime", &ParticleSystem::orbitOverLifetime)
		.property("orbitAxis", &ParticleSystem::orbitAxis)
		.property("startOrbitVelocity", &ParticleSystem::startOrbitVelocity)
		.property("endOrbitVelocity", &ParticleSystem::endOrbitVelocity)
		.property("glowValueType", &ParticleSystem::glowValueType)
		.property("glow", &ParticleSystem::glow)
		.property("glowIntensity", &ParticleSystem::glowIntensity)
		.property("minGlowIntensity", &ParticleSystem::minGlowIntensity)
		.property("maxGlowIntensity", &ParticleSystem::maxGlowIntensity)
		.property("alwaysFaceCamera", &ParticleSystem::alwaysFaceCamera)
		.property("ignoreLights", &ParticleSystem::ignoreLights)
		.property("particleLayer", &ParticleSystem::particleLayer)
		.property("renderMode", &ParticleSystem::renderMode)
		.property("textureGUID", &ParticleSystem::textureGUID)
		.property("textureHandle", &ParticleSystem::textureHandle)
		.property("modelHandle", &ParticleSystem::modelHandle)
		.property("materialHandle", &ParticleSystem::materialHandle);

	rttr::registration::class_<ParticleSystem::Burst>(typeid(ParticleSystem::Burst).name())
		.constructor<>()
		.property("numParticles", &ParticleSystem::Burst::numParticles)
		.property("burstRepetitions", &ParticleSystem::Burst::burstRepetitions)
		.property("burstPeriod", &ParticleSystem::Burst::burstPeriod)
		.property("triggerTime", &ParticleSystem::Burst::triggerTime)
		.property("triggered", &ParticleSystem::Burst::triggered)
		(
			rttr::metadata("Serialize", false)
		);

	rttr::registration::class_<std::vector<ParticleSystem::Burst>>("BurstVector");

	rttr::registration::class_<Particle>(typeid(Particle).name())
		.constructor<>()
		.property("particles", &ParticleSystem::particles)
		(
			rttr::metadata("Serialize", false)
		);

	rttr::registration::class_<std::vector<Particle>>("vector<Particle>")
		(
			rttr::metadata("Serialize", false)
		);


	rttr::registration::class_<Animator>(typeid(Animator).name())
		.constructor<>()
		.property("current_time", &Animator::current_time)
		.property("stateMachine Handle", &Animator::Handle_stateMachine)
		.property("AnimPkg Handle", &Animator::Handle_curr_anim_pkg)
		.property("Skeleton Handle", &Animator::Handle_skeleton)
		.property("componentEnabled", &Animator::componentEnabled)
		.property("Anims Pkg GUID", &Animator::Handle_Anims)
		.property("eventFrames", &Animator::eventFrames);


	rttr::registration::class_<Bone>(typeid(Bone).name())
		.constructor<>()
		.property("skeleton_root", &Bone::skeleton_root)
		.property("frame_idx", &Bone::frame_idx);

	rttr::registration::class_<Canvas>(typeid(Canvas).name())
		.constructor<>()
		.property("canvas_type", &Canvas::canvas_type)
		.property("sort_order", &Canvas::sort_order)
		.property("graphics_raycast", &Canvas::graphic_raycastable)
		.property("componentEnabled", &Canvas::componentEnabled)
		.property("billboardX", &Canvas::billboardX)
		.property("billboardY", &Canvas::billboardY);

	rttr::registration::class_<Button>(typeid(Button).name())
		.constructor<>()
		.property("transition", &Button::transition)
		.property("color_tints", &Button::color_transitions)
		.property("sprite_state", &Button::sprite_transitions)
		.property("componentEnabled", &Button::componentEnabled);

	rttr::registration::class_<Slider>(typeid(Slider).name())
		.constructor<>()
		.property("Axis", &Slider::axis)
		.property("Direction", &Slider::direction)
		.property("handle", &Slider::handle)
		.property("fill", &Slider::fill)
		.property("value", &Slider::value)
		.property("contained", &Slider::contained)
		.property("enabled", &Slider::componentEnabled);

rttr::registration::class_<RectTransform>(typeid(RectTransform).name())
	.constructor<>()
	.property("hori_pivot", &RectTransform::hori_pivot)
	.property("vert_pivot", &RectTransform::vert_pivot)
	.property("pos_x", &RectTransform::pos_x)
	.property("pos_y", &RectTransform::pos_y)
	.property("left", &RectTransform::left)
	.property("right", &RectTransform::right)
	.property("top", &RectTransform::top)
	.property("bot", &RectTransform::bot)
	.property("width", &RectTransform::width)
	.property("height", &RectTransform::height)
	.property("rotation", &RectTransform::final_rot);

rttr::registration::class_<SpriteRenderer>(typeid(SpriteRenderer).name())
.constructor<>()
.property("texture", &SpriteRenderer::textureHandle)
.property("rgba", &SpriteRenderer::rgba)
.property("alphathreshold", &SpriteRenderer::alphathreshold)
.property("raycast_target", &SpriteRenderer::raycast_target)
.property("componentEnabled", &SpriteRenderer::componentEnabled);

rttr::registration::class_<SpriteRendererGammaOverride>(typeid(SpriteRendererGammaOverride).name())
.constructor<>()
.property("gamma", &SpriteRendererGammaOverride::gamma)
.property("componentEnabled", &SpriteRendererGammaOverride::componentEnabled);

rttr::registration::class_<SpriteAnimator>(typeid(SpriteAnimator).name())
.constructor<>()
.property("is_playing", &SpriteAnimator::is_playing)
.property("loop", &SpriteAnimator::loop)
.property("fps", &SpriteAnimator::fps)
.property("row", &SpriteAnimator::row)
.property("col", &SpriteAnimator::col)
.property("num_frames", &SpriteAnimator::num_frames)
.property("componentEnabled", &SpriteAnimator::componentEnabled);

rttr::registration::class_<FontRenderer>(typeid(FontRenderer).name())
.constructor<>()
.property("font", &FontRenderer::fontHandle)
.property("rgba", &FontRenderer::rgba)
.property("font_size", &FontRenderer::font_size)
.property("line_spacing", &FontRenderer::line_spacing)
.property("alignment", &FontRenderer::alignment)
.property("text", &FontRenderer::text)
.property("offsetx", &FontRenderer::offset_x)
.property("offsety", &FontRenderer::offset_y)
.property("componentEnabled", &FontRenderer::componentEnabled);


//rttr::registration::class_<NavMeshLink>(typeid(NavMeshLink).name())
//.constructor<>()
//.property("startLink", &NavMeshLink::startLink)
//.property("endLink", &NavMeshLink::endLink)
//.property("bidirectional", &NavMeshLink::bidirectional)
//.property("currentPath", &NavMeshLink::radius);


rttr::registration::class_<Prefab>(typeid(Prefab).name())
.constructor<>()
.property("prefabID", &Prefab::prefabID)
.property("prefabGUID", &Prefab::prefabGUID)
.property("prefabHandle", &Prefab::prefabHandle);
	}
}
#pragma endregion

namespace SliceEngine
{
	// forward declare global pointer to action mapping system
	//extern ActionMappingSystem* gActionMappingSystemInstance;
	//// actual single instance pointer which is static
	//static ActionMappingSystem actionMapSystemInstance(Core::GetInstance()->GetInputSystem());

	// removed this from engine.cpp because core.cpp now has the global action mapping system instance ptr
	namespace
	{
		static bool isPlaying = false;
	}

	//Time class for physics simulation or any other system that uses fixeddt
	void EnableMemoryLeakChecking(int breakAlloc = -1)
	{
		int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
		tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
		_CrtSetDbgFlag(tmpDbgFlag);

		if (breakAlloc != -1) _CrtSetBreakAlloc(breakAlloc);
	}

	Engine::Engine()
	{
		isRunning = false;
	}
	Engine::~Engine()
	{
	}

	void Engine::Test()
	{
		std::cout << " Hi from Engine Test Function\n";
	}



	void Engine::Init()
	{
		EnableMemoryLeakChecking(-1);

		SLICE_LOG("Initializing Slice Engine.");
		glfwInit();

		Core::GetInstance()->InitCore();


		// Set up Engine Systems
		isRunning = true;
		//auto window = Core::GetInstance()->GetWindow();
		Core::GetInstance()->GetWindow();

		// mResource = std::make_unique<ResourceManager>();
		//frm.Init();
		frm = Core::GetInstance()->GetFramerateManager();
		frm->Init();

		auto mAudioManager = Core::GetInstance()->GetAudioManager();

		//audio->LoadSound("Assets/Audio/BGM_MainMenu_Mix1.wav");
		mAudioManager->Init();

		FactoryInstance.InitRootEntity();
		Core::GetInstance()->InitSystem<AudioSourceSystem>();
		Core::GetInstance()->InitSystem<AudioListenerSystem>();
		Core::GetInstance()->InitSystem<WorldSpaceGraphicsSystem>();
		Core::GetInstance()->InitSystem<LightingSystem>();
		Core::GetInstance()->InitSystem<TransformSystem>();

		Core::GetInstance()->InitSystem<CanvasSystem>();
		Core::GetInstance()->InitSystem<SpriteAnimationSystem>();
		Core::GetInstance()->InitSystem<ButtonSystem>();
		Core::GetInstance()->InitSystem<SliderSystem>();

		Core::GetInstance()->InitSystem<ParticleSystemManager>();
		Core::GetInstance()->InitSystem<PrefabSystem>();
		//Core::GetInstance()->InitSystem<NetworkSystem>();
		Core::GetInstance()->InitSystem<AnimatorSystem>();
		Core::GetInstance()->InitSystem<BoneSystem>();


		Core::GetInstance()->InitSystem<PhysicsSystem>();
		Core::GetInstance()->InitSystem<ScriptSystem>();
		Core::GetInstance()->GetSystem<PhysicsSystem>().Initialize();
		Core::GetInstance()->GetSystem<PhysicsSystem>().SubscribeToEvents();
		Core::GetInstance()->GetSystem<AudioSourceSystem>().BindToAudioSource();
		Core::GetInstance()->GetSystem<AudioListenerSystem>().BindToAudioListener();
		Core::GetInstance()->GetLayerManager()->Init();
		Core::GetInstance()->GetSceneSystem()->Init();

		gScriptSystem->Init();
		//audio->PlaySound("BGM_MainMenu_Mix1", SliceEngine::SoundCategory::BGM, SliceEngine::AudioManager::InternalSound::SOUND_BGM, false, false, 0.5f);
		//audio->PlaySound("3DAudioTest", SliceEngine::SoundCategory::BGM, SliceEngine::AudioManager::InternalSound::SOUND_BGM, true, false, 0.5f);

		//auto mResource = Core::GetInstance()->GetResourceManager();
		Core::GetInstance()->GetResourceManager();
		auto mRender = Core::GetInstance()->GetRenderManager();
		Core::GetInstance()->InitSystem<CameraSystem>();

		mRender->CreateInstancingParams();
		mRender->CreateDeferredTextures();

		Core::GetInstance()->GetSystem<PrefabSystem>().InitEvent();

		Core::GetInstance()->GetProjectSettingsManager()->Init();

		// =========================== TESTING AREA ===========================
		// 
		//mRender->CreateCamera();

		auto& mCanvas = Core::GetInstance()->GetSystem<CanvasSystem>();
		mCanvas.Init();

		auto& sButton = Core::GetInstance()->GetSystem<ButtonSystem>();
		sButton.InitSystem();
		//entt::entity newCam = Core::GetInstance()->GetRegistry().create();
		//Core::GetInstance()->GetRegistry().emplace<Transform>(newCam);
		//Core::GetInstance()->GetRegistry().emplace<Renderer>(newCam);
		//auto mNetwork = Core::GetInstance()->GetNetwork();
		//mNetwork->Init();


		EventManager::GetInstance()->Subscribe<OnSceneChangeEvent, &Engine::SceneChangeEvent>(this);
	}

	void Engine::WindowSizeSwitch()
	{
		auto sInputs = SliceEngine::Core::GetInstance()->GetInputSystem();
		auto windowManager = SliceEngine::Core::GetInstance()->GetWindowManager();

		if (sInputs->IsKeyDown(GLFW_KEY_RIGHT_ALT))
		{
			if (sInputs->IsKeyPressed(GLFW_KEY_ENTER))
			{
				if (windowManager->isFullScreen)
				{
					windowManager->NonFullScreenWindow();
				}
				else
				{
					windowManager->FullScreenWindow();
				}
			}

		}
	}

	void Engine::Update()
	{
		frm->StartSystem("Misc");
		auto core = Core::GetInstance();
		auto sScene = Core::GetInstance()->GetSceneSystem();
		auto sRender = core->GetRenderManager();
		auto sAudio = core->GetAudioManager();
		//auto sInputs = core->GetInputSystem();
		auto projSettingsManager = core->GetProjectSettingsManager();

		auto& sTransform = core->GetSystem<TransformSystem>();
		//auto& sAnimator = core->GetSystem<AnimatorSystem>();
		//auto& sBone = core->GetSystem<BoneSystem>();
		auto& sCanvas = core->GetSystem<CanvasSystem>();
		//auto& sButton = core->GetSystem<ButtonSystem>();
		//auto& sSlider = core->GetSystem<SliderSystem>();
		//auto& sNav = core->GetSystem<NavigationSystem>();
		auto& prefabSys = core->GetSystem<PrefabSystem>();
		auto& sParticleSystemManager = core->GetSystem<ParticleSystemManager>();

		(void)projSettingsManager;
		(void)sParticleSystemManager;

		if (!sScene->CheckQueueEmpty())
		{
			if (sScene->isSceneUnloaded)
			{
				sScene->LoadSceneFromQueue();
			}
		}

		while (sScene->mCurrentState != sScene->mNextState)
		{
			if (sScene->mNextState == SceneState::PLAY_SCENE)
			{
				OnPlayStart();
			}

			if (sScene->mNextState == SceneState::PAUSE_SCENE)
			{
				OnPauseStart();
			}

			if (sScene->mNextState == SceneState::STOP_SCENE)
			{
				OnStopStart();
			}
		}

		frm->EndSystem("Misc");
		frm->StartSystem("Update Delta Time");
		frm->updateDeltaTime();

		deltaTimeUnscaled = static_cast<float>(frm->getDeltaTime());
		deltaTimeScaled = deltaTimeUnscaled * sScene->GetTimeScale();
		fixedDeltaTime = static_cast<float>(frm->getFixedDeltaTime());
		fixedDeltaTimeScaled = fixedDeltaTime * sScene->GetTimeScale();
		frm->EndSystem("Update Delta Time");

		frm->StartSystem("Script");
		gScriptSystem->UpdateScripts();
		frm->EndSystem("Script");

		// Run Simulation (Physics, Animations, FixedUpdate Scripts)
		if (sScene->mCurrentState == SceneState::PLAY_SCENE)
		{
			OnPlayStarted();
		}

		// regular transform update
		frm->StartSystem("Transform");
		sTransform.Update(deltaTimeScaled);
		sTransform.UpdateTransforms();
		prefabSys.UpdateBasePrefabs();
		frm->EndSystem("Transform");

		frm->StartSystem("Canvas");
		sCanvas.UpdateHierachy();
		frm->EndSystem("Canvas");

		frm->StartSystem("Audio");
		core->GetSystem<AudioSourceSystem>().Update(deltaTimeUnscaled);
		core->GetSystem<AudioListenerSystem>().Update(deltaTimeUnscaled);
		sAudio->Update();
		frm->EndSystem("Audio");


		// note: might need to have a physics update version of particle sys to call in fixedDT loop
		/*if (sScene->mCurrentState == SceneState::PLAY_SCENE)
		{
			OnPlayStarted();
		}*/

		frm->StartSystem("Particle System");
		core->GetSystem<ParticleSystemManager>().Update(deltaTimeScaled);
		frm->EndSystem("Particle System");

		frm->StartSystem("Graphics");
		sRender->Update(deltaTimeScaled);
		sRender->Render();
		sCanvas.DrawOverlay();
		frm->EndSystem("Graphics");
	}

	void Engine::SceneChangeEvent(const OnSceneChangeEvent& event)
	{
		auto core = Core::GetInstance();
		auto sAudio = core->GetAudioManager();
		auto sInputs = core->GetInputSystem();
		auto projSettingsManager = core->GetProjectSettingsManager();

		auto& sButton = core->GetSystem<ButtonSystem>();
		sButton.InitSystem();
		auto& sParticleSystemManager = core->GetSystem<ParticleSystemManager>();
		(void)sParticleSystemManager;

		core->GetSystem<PhysicsSystem>().ClearCollisionPairs();
		sInputs->SetEnabled(false);
		sInputs->ResetCursorState();
		sAudio->StopAllSound();
		auto audioSettings = projSettingsManager->GetSettings<AudioSettings>();
		audioSettings->DeleteAM();
		isPlaying = false;
		gScriptSystem->OnEnd();

	}

	void Engine::OnPlayStart()
	{
		auto core = Core::GetInstance();
		auto sInputs = core->GetInputSystem();
		auto sScene = core->GetSceneSystem();
		auto& sAnimator = core->GetSystem<AnimatorSystem>();
		auto& sButton = core->GetSystem<ButtonSystem>();
		auto& sCanvas = core->GetSystem<CanvasSystem>();
		auto sAudio = core->GetAudioManager();

		sInputs->SetMode(InputMode::Game);
		sInputs->SetEnabled(true);
		if (sScene->mCurrentState == SceneState::DEFAULT)
		{
			sScene->WriteTempFile();
		}

		if (sScene->mCurrentState == SceneState::PAUSE_SCENE)
		{
			sAudio->SetCategoryPause(0, false);
			sAudio->SetCategoryPause(1, false);
		}

		if (!isPlaying)
		{
			sCanvas.UpdateHierachy(true);	//force all ui components to update once regardless of inactive
			SliceEngine::gScriptSystem->OnStart();
			sAnimator.InitSystem();
			sButton.InitSystem();
			sCanvas.UpdateHierachy(true);
			FactoryInstance.CreateGO("AudioManager");
			isPlaying = true;
		}

		sScene->mCurrentState = SceneState::PLAY_SCENE;

	}

	void Engine::OnStopStart()
	{
		auto core = Core::GetInstance();
		auto sScene = core->GetSceneSystem();

		sScene->ReloadScene();
		sScene->mCurrentState = SceneState::RELOAD_SCENE;
		sScene->mNextState = SceneState::RELOAD_SCENE;
	}

	void Engine::OnPauseStart()
	{
		auto core = Core::GetInstance();
		auto sInputs = core->GetInputSystem();
		auto sAudio = core->GetAudioManager();
		auto sScene = core->GetSceneSystem();

		sInputs->SetEnabled(false);
		sAudio->SetCategoryPause(0, true);
		sAudio->SetCategoryPause(1, true);
		//isPlaying = false;
		sScene->mCurrentState = SceneState::PAUSE_SCENE;
	}

	void Engine::OnPlayStarted()
	{
		auto core = Core::GetInstance();
		auto& sTransform = core->GetSystem<TransformSystem>();
		auto& sAnimator = core->GetSystem<AnimatorSystem>();
		auto& sBone = core->GetSystem<BoneSystem>();
		auto sInputs = core->GetInputSystem();
		auto& sCanvas = core->GetSystem<CanvasSystem>();
		auto& sButton = core->GetSystem<ButtonSystem>();
		auto& sSlider = core->GetSystem<SliderSystem>();
		auto& sSpriteAnim = core->GetSystem<SpriteAnimationSystem>();

		//frm->StartSystem("Fixed Dt Loop");
		for (size_t step = 0; step < frm->getCurrentNumberOfSteps(); ++step)
		{
			// game logic
			frm->StartSystem("Script");
			gScriptSystem->OnFixedUpdate(fixedDeltaTimeScaled);
			frm->EndSystem("Script");

			frm->StartSystem("Transform");
			// sync matrices before physics step
			sTransform.Update(fixedDeltaTimeScaled);
			sTransform.UpdateTransforms();
			frm->EndSystem("Transform");

			// physics update
			frm->StartSystem("Physics");
			core->GetSystem<PhysicsSystem>().PreStepSync();
			core->GetSystem<PhysicsSystem>().StepWorld(fixedDeltaTimeScaled);
			core->GetSystem<PhysicsSystem>().PostStepSync();
			frm->EndSystem("Physics");

			frm->StartSystem("Transform");
			// sync matrices after physics
			sTransform.PostStepSyncTransforms(Core::FactoryInstance.GetRootEntity(), glm::mat4(1.0f));
			//	sTransform.UpdateTransforms();	//not needed since the above line resolves local and world
			frm->EndSystem("Transform");


			// animation after logic and physics
			frm->StartSystem("Animation");
			sAnimator.Update(fixedDeltaTimeScaled);
			sBone.Update_Scenegraph();
			sAnimator.BoneUpdate();
			frm->EndSystem("Animation");
		}
		//frm->EndSystem("Fixed Dt Loop");

		// regular update for scripts
		// idk if this should be before or after simulation loop
		frm->StartSystem("Script");
		gScriptSystem->OnUpdate(deltaTimeScaled);
		frm->EndSystem("Script");



		frm->StartSystem("Sprite Animation");
		sSpriteAnim.Update(deltaTimeUnscaled);
		frm->EndSystem("Sprite Animation");

		frm->StartSystem("Canvas");
		//glm::vec2 mouse_coord = sInputs->GetMousePosition();
		glm::vec2 mouse_NDC = sInputs->GetMouseNDC();
		//for now im just gona directly convert to game screen coord
		unsigned int mouse_x = static_cast<unsigned int>(mouse_NDC.x * CanvasSystem::target_width);//(unsigned int)mouse_coord.x;
		unsigned int mouse_y = static_cast<unsigned int>(CanvasSystem::target_height - mouse_NDC.y * CanvasSystem::target_height);// (unsigned int)mouse_coord.y;
		Entity raycast_target = sCanvas.Raycast(mouse_x, mouse_y);
		frm->EndSystem("Canvas");
		//	std::cout << "raycast: " << (unsigned int)raycast_target << std::endl;
		frm->StartSystem("UI Interaction");
		sButton.HandleMouse(*sInputs, raycast_target);
		sSlider.HandleMouse(*sInputs, raycast_target);
		frm->EndSystem("UI Interaction");

		// late update for scripts
		frm->StartSystem("Script");
		gScriptSystem->OnLateUpdate(deltaTimeScaled);
		frm->EndSystem("Script");
	}

	void Engine::Draw()
	{
		Core::GetInstance()->GetRenderManager()->Draw();
	}

	void Engine::EndFrame()
	{
		frm->StartSystem("Update Destroyed");
		Core::FactoryInstance.UpdateDestroyed();
		Core::GetInstance()->GetSceneSystem()->isSceneUnloaded = true;
		frm->EndSystem("Update Destroyed");

		frm->StartSystem("GLFW Swap Buffers");
		auto window = Core::GetInstance()->GetWindow();
		if (glfwWindowShouldClose(window))
			isRunning = false;
		//auto inputs = Core::GetInstance()->GetInputSystem();
		glfwSwapBuffers(window);
		frm->EndSystem("GLFW Swap Buffers");
	}

	void Engine::Exit()
	{
		auto& mCanvas = Core::GetInstance()->GetSystem<CanvasSystem>();
		mCanvas.Release();

		Core::GetInstance()->ExitCore();

		SLICE_LOG("Shutting Down Slice Engine.");
	}

}