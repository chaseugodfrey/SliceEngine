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
#include "Navigation/NavigationSystem.h"
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


	register_std_array<uint32_t, 4>("Array4UInt32");
	register_std_array<Entity, 4>("Array4Entity");

	rttr::registration::class_<glm::vec2>("glm::vec2")
		.constructor<>()(rttr::policy::ctor::as_object)
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

	rttr::registration::class_<ColliderShape>(typeid(ColliderShape).name())
		.constructor<>()
		.property("boxData", &ColliderShape::GetBoxData, &ColliderShape::SetBoxData)
		.property("sphereData", &ColliderShape::GetSphereData, &ColliderShape::SetSphereData)
		.property("capsuleData", &ColliderShape::GetCapsuleData, &ColliderShape::SetCapsuleData)
		.property("offSet", &ColliderShape::offSet)
		.property("isTrigger", &ColliderShape::isTrigger)
		.property("componentEnabled", &ColliderShape::componentEnabled);

	rttr::registration::class_<Renderer>(typeid(Renderer).name())
		.constructor<>()
		.property("model", &Renderer::modelHandle)
		.property("material", &Renderer::materialHandle)
		.property("renderTag", &Renderer::renderTag)
		.property("skinned", &Renderer::skinned) // If i do this, i'll need to serialize bone info and animator component
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
		.property("spread", &AudioSource::spread)
		.property("minDistance", &AudioSource::minDistance)
		.property("maxDistance", &AudioSource::maxDistance)
		.property("priority", &AudioSource::priority)
		.property("playOnAwake", &AudioSource::playOnAwake)
		.property("volumeRollOff", &AudioSource::volumeRollOff)
		.property("playPreview", &AudioSource::playPreview)
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
		.property("fogColor", &Camera::fogColor)
		.property("fogIntensity", &Camera::fogIntensity)
		.property("bloomStrength", &Camera::bloomStrength)
		.property("bloomFilterRadius", &Camera::bloomFilterRadius)
		.property("bloomExposure", &Camera::exposure)
		.property("vignetteCenter", &Camera::vignetteCenter)
		.property("vignetteIntensity", &Camera::vignetteIntensity)
		.property("vignetteSmoothness", &Camera::vignetteSmoothness)
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
			rttr::value("Overlay", Canvas::Type::OVERLAY)
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
			rttr::value("Logarithmic", AudioSource::VolumeRollOff::Linear)
		);
	rttr::registration::enumeration<AudioSource::Category>("Category")
		(
			rttr::value("SFX", AudioSource::Category::SFX),
			rttr::value("BGM", AudioSource::Category::BGM),
			rttr::value("UI", AudioSource::Category::UI),
			rttr::value("EditorSounds", AudioSource::Category::EditorSounds)
		);
	rttr::registration::class_<Light>(typeid(Light).name())
		.constructor<>()
		.property("type", &Light::type)
		.property("color", &Light::color)
		.property("intensity", &Light::intensity)
		.property("componentEnabled", &Light::componentEnabled);

	rttr::registration::class_<GUID>("GUID")
		.constructor<>()
		.constructor<uint64_t>()
		.property_readonly("Value", &GUID::GetGUID);

	rttr::registration::enumeration<ParticleSystem::ShapeType>(typeid(ParticleSystem::ShapeType).name())
		(
			rttr::value("CONE", ParticleSystem::ShapeType::CONE),
			rttr::value("SPHERE", ParticleSystem::ShapeType::SPHERE),
			rttr::value("BOX", ParticleSystem::ShapeType::BOX),
			rttr::value("EDGE", ParticleSystem::ShapeType::EDGE),
			rttr::value("CIRCLE", ParticleSystem::ShapeType::CIRCLE),
			rttr::value("RECTANGLE", ParticleSystem::ShapeType::RECTANGLE)
			);

	rttr::registration::class_<Particle>(typeid(Particle).name())
		.constructor<>()
		.property("active", &Particle::active)

		.property("age", &Particle::age)
		.property("rotation", &Particle::rotation)
		.property("speed", &Particle::speed)

		.property("position", &Particle::position)
		.property("scale", &Particle::scale)
		.property("velocity", &Particle::velocity)
		.property("colour", &Particle::colour);

	rttr::registration::class_<ParticleSystem>(typeid(ParticleSystem).name())
		.constructor<>()
		.property("duration", &ParticleSystem::duration)
		.property("isRepeating", &ParticleSystem::isRepeating)
		.property("isLocalSpace", &ParticleSystem::isLocalSpace)

		.property("destroyOnExpire", &ParticleSystem::destroyOnExpire)
		.property("maxParticles", &ParticleSystem::maxParticles)

		.property("gForce", &ParticleSystem::gForce)
		.property("emissionRate", &ParticleSystem::emissionRate)

		.property("bursts", &ParticleSystem::bursts)

		.property("shapeType", &ParticleSystem::shapeType)

		.property("coneArc", &ParticleSystem::coneArc)
		.property("coneRadius", &ParticleSystem::coneRadius)

		.property("shapeRadius", &ParticleSystem::sphereRadius)
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

		.property("colourOverLifetime", &ParticleSystem::colourOverLifetime)
		.property("colour", &ParticleSystem::colourLifeTimeMap)
		
		// temp
		.property("colourOverLifetimeEnd", &ParticleSystem::colourOverLifetimeEnd)
		.property("hasCollision", &ParticleSystem::hasCollision)

		.property("textureGUID", &ParticleSystem::textureGUID)
		.property("materialGUID", &ParticleSystem::materialGUID)
		.property("meshGUID", &ParticleSystem::meshGUID)
		;

	rttr::registration::class_<ParticleSystem::Burst>(typeid(ParticleSystem::Burst).name())
		.constructor<>()
		.property("numParticles", &ParticleSystem::Burst::numParticles)
		.property("burstRepetitions", &ParticleSystem::Burst::burstRepetitions)
		.property("burstPeriod", &ParticleSystem::Burst::burstPeriod)
		.property("triggerTime", &ParticleSystem::Burst::triggerTime)
		.property("triggered", &ParticleSystem::Burst::triggered);

	rttr::registration::class_<std::vector<ParticleSystem::Burst>>("BurstVector");

	rttr::registration::class_<Particle>(typeid(Particle).name())
		.constructor<>()
		.property("particles", &ParticleSystem::particles)
		(
			rttr::metadata("Serialize", false)
		);


	rttr::registration::class_<std::vector<Particle>>("vector<Particle>");

	rttr::registration::class_<Animator>(typeid(Animator).name())
		.constructor<>()
		.property("current_time", &Animator::current_time)
		.property("stateMachine Handle", &Animator::Handle_stateMachine)
		.property("AnimPkg Handle", &Animator::Handle_curr_anim_pkg)
		.property("Skeleton Handle", &Animator::Handle_skeleton)
		.property("componentEnabled", &Animator::componentEnabled);


	rttr::registration::class_<Bone>(typeid(Bone).name())
		.constructor<>()
		.property("skeleton_root", &Bone::skeleton_root)
		.property("frame_idx", &Bone::frame_idx);

	rttr::registration::class_<Canvas>(typeid(Canvas).name())
		.constructor<>()
		.property("canvas_type", &Canvas::canvas_type)
		.property("sort_order", &Canvas::sort_order)
		.property("graphics_raycast", &Canvas::graphic_raycastable)
		.property("componentEnabled", &Canvas::componentEnabled);

	rttr::registration::class_<Button>(typeid(Button).name())
		.constructor<>()
		.property("transition", &Button::transition)
		.property("componentEnabled", &Button::componentEnabled);

	rttr::registration::class_<Slider>(typeid(Slider).name())
		.constructor<>()
		.property("Axis", &Slider::axis)
		.property("Direction", &Slider::direction)
		.property("handle", &Slider::handle)
		.property("fill", &Slider::fill)
		.property("value", &Slider::value)
		.property("enabled", &Slider::componentEnabled);
	//.property("colors", &Button::color_transitions)
	//.property("sprites", &Button::sprite_transitions);

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
	.property("height", &RectTransform::height);

rttr::registration::class_<SpriteRenderer>(typeid(SpriteRenderer).name())
.constructor<>()
.property("texture", &SpriteRenderer::textureHandle)
.property("rgba", &SpriteRenderer::rgba)
.property("raycast_target", &SpriteRenderer::raycast_target)
.property("componentEnabled", &SpriteRenderer::componentEnabled);

rttr::registration::class_<NavAgent>(typeid(NavAgent).name())
	.constructor<>()
	.property("speed", &NavAgent::speed)
	.property("target", &NavAgent::target)
	.property("hasNewTarget", &NavAgent::hasNewTarget)
	.property("currentPath", &NavAgent::currentPath)
	.property("currentPathIndex", &NavAgent::currentPathIndex)
	.property("componentEnabled", &NavAgent::componentEnabled);

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
		Core::GetInstance()->InitSystem<ButtonSystem>();
		Core::GetInstance()->InitSystem<SliderSystem>();

		Core::GetInstance()->InitSystem<ParticleSystemManager>();
		Core::GetInstance()->InitSystem<PrefabSystem>();
		//Core::GetInstance()->InitSystem<NetworkSystem>();
		Core::GetInstance()->InitSystem<AnimatorSystem>();
		Core::GetInstance()->InitSystem<BoneSystem>();
		Core::GetInstance()->InitSystem<NavigationSystem>();


		Core::GetInstance()->InitSystem<PhysicsSystem>();
		Core::GetInstance()->InitSystem<ScriptSystem>();
		Core::GetInstance()->GetSystem<PhysicsSystem>().Initialize();
		Core::GetInstance()->GetSystem<PhysicsSystem>().SubscribeToEvents();
		Core::GetInstance()->GetSystem<AudioSourceSystem>().BindToAudioSource();
		Core::GetInstance()->GetSystem<AudioListenerSystem>().BindToAudioListener();
		Core::GetInstance()->GetLayerManager()->Init();
		Core::GetInstance()->GetSystem<NavigationSystem>().Init();

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
		Core::GetInstance()->GetSceneSystem()->Init();

		// =========================== TESTING AREA ===========================
		// 
		//mRender->CreateCamera();

		auto& mCanvas = Core::GetInstance()->GetSystem<CanvasSystem>();
		mCanvas.Init();

		auto& sButton = Core::GetInstance()->GetSystem<ButtonSystem>();
		//sButton.Init();
		//entt::entity newCam = Core::GetInstance()->GetRegistry().create();
		//Core::GetInstance()->GetRegistry().emplace<Transform>(newCam);
		//Core::GetInstance()->GetRegistry().emplace<Renderer>(newCam);
		//auto mNetwork = Core::GetInstance()->GetNetwork();
		//mNetwork->Init();


	}


	void Engine::Update()
	{
		auto core = Core::GetInstance();
		auto sScene = Core::GetInstance()->GetSceneSystem();
		auto sRender = core->GetRenderManager();
		auto sAudio = core->GetAudioManager();
		auto sInputs = core->GetInputSystem();
		auto projSettingsManager = core->GetProjectSettingsManager();
		auto& sTransform = core->GetSystem<TransformSystem>();
		auto& sAnimator = core->GetSystem<AnimatorSystem>();
		auto& sBone = core->GetSystem<BoneSystem>();
		auto& sCanvas = core->GetSystem<CanvasSystem>();
		auto& sButton = core->GetSystem<ButtonSystem>();
		auto& sSlider = core->GetSystem<SliderSystem>();
		auto& sNav = core->GetSystem<NavigationSystem>();
		auto& prefabSys = core->GetSystem<PrefabSystem>();
		auto& sParticleSystemManager = core->GetSystem<ParticleSystemManager>();

		static bool isPlaying = false;

		if (!sScene->CheckQueueEmpty())
		{
			if (sScene->isSceneUnloaded)
			{
				sScene->LoadNextScene();
			}
		}

		while (sScene->mCurrentState != sScene->mNextState)
		{
			//Line to load resources
			if (sScene->mNextState == SceneState::PLAY_SCENE)
			{
				sInputs->SetMode(InputMode::Game);
				sInputs->SetEnabled(true);
				if (sScene->mCurrentState == SceneState::DEFAULT)
				{
					sScene->WriteTempFile();
				}

				if (!isPlaying)
				{
					SliceEngine::gScriptSystem->OnStart();
					sAnimator.InitSystem();
					sButton.InitSystem();
					FactoryInstance.CreateGO("AudioManager");
					isPlaying = true;
				}

				if (sScene->mCurrentState == SceneState::PAUSE_SCENE)
				{
					sAudio->SetCategoryPause(0, false);
					sAudio->SetCategoryPause(1, false);
				}

				sScene->mCurrentState = SceneState::PLAY_SCENE;
			}

			if (sScene->mNextState == SceneState::PAUSE_SCENE)
			{
				sInputs->SetMode(InputMode::Editor);
				sInputs->SetEnabled(false);
				sAudio->SetCategoryPause(0, true);
				sAudio->SetCategoryPause(1, true);
				//isPlaying = false;
				sScene->mCurrentState = SceneState::PAUSE_SCENE;
			}

			//When the stop button has been clicked and the scene state is set to STOP_SCENE, reload the current scene
			if (sScene->mNextState == SceneState::STOP_SCENE)
			{
				sInputs->SetMode(InputMode::Editor);
				sInputs->SetEnabled(false);
				sInputs->ResetCursorState();
				sParticleSystemManager.ResetManager();
				sAudio->StopAllSound();
				auto audioSettings = projSettingsManager->GetSettings<AudioSettings>();
				audioSettings->DeleteAM();
				
				sScene->ReloadScene();
				isPlaying = false;

				gScriptSystem->OnEnd();

				sScene->mCurrentState = SceneState::DEFAULT;
				sScene->mNextState = SceneState::DEFAULT;
			}
		}

		frm->updateDeltaTime(); //update deltatime and currentnumber of steps for systems that uses fixeddt
		frm->StartFrame();

		frm->StartSystem("GLFW Poll Events");
		glfwMakeContextCurrent(core->GetWindow());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwPollEvents();
		frm->EndSystem("GLFW Poll Events");

		frm->StartSystem("Input");
		sInputs->UpdatePrevInput();
		GetActionMappingSystem().processAllInput();
		frm->EndSystem("Input");

		// process all enabled action maps in Game mode
		if (sScene->mCurrentState == SceneState::PLAY_SCENE)
		{
			SliceEngine::GetActionMappingSystem().processAllInput();
		}

		frm->StartSystem("Audio");
		core->GetSystem<AudioSourceSystem>().Update(static_cast<float>(frm->getDeltaTime()));
		core->GetSystem<AudioListenerSystem>().Update(static_cast<float>(frm->getDeltaTime()));
		sAudio->Update();
		frm->EndSystem("Audio");

		frm->StartSystem("Script");
		gScriptSystem->UpdateScripts();
		gScriptSystem->Update((float)frm->getDeltaTime());
		if (sScene->mCurrentState == SceneState::PLAY_SCENE)
		{
			gScriptSystem->OnUpdate((float)frm->getDeltaTime());
		}
		frm->EndSystem("Script");


		frm->StartSystem("Transform");
		sTransform.Update(static_cast<float>(frm->getFixedDeltaTime()));
		sTransform.UpdateTransforms();
		prefabSys.UpdateBasePrefabs(); // updates base prefab transform so ig it belongs here idk
		frm->EndSystem("Transform");

		if (sScene->mCurrentState == SceneState::PLAY_SCENE)
		{
			for (size_t step = 0; step < frm->getCurrentNumberOfSteps(); ++step)
			{
				frm->StartSystem("Physics");

				core->GetSystem<PhysicsSystem>().Update(static_cast<float>(frm->getFixedDeltaTime()));

				// Single world step
				core->GetSystem<PhysicsSystem>().StepWorld(static_cast<float>(frm->getFixedDeltaTime()));

				// Post-step: pull dynamic poses for rendering
				core->GetSystem<PhysicsSystem>().PostStepSync();
				frm->EndSystem("Physics");

				//sTransform.UpdateTransforms();

			}
			sTransform.PostStepSyncTransforms(Core::FactoryInstance.GetRootEntity(), glm::mat4(1.0f));

		}

		if (sScene->mCurrentState == SceneState::PLAY_SCENE)
		{
			for (size_t step = 0; step < frm->getCurrentNumberOfSteps(); ++step)
			{
				sAnimator.Update(static_cast<float>(frm->getFixedDeltaTime()));
				sBone.Update_Scenegraph();
				sAnimator.BoneUpdate();
			}

			//somehow convert to pixel coord
			glm::vec2 mouse_coord = sInputs->GetMousePosition();
			//for now im just gona directly convert to game screen coord
			unsigned int mouse_x = (unsigned int)mouse_coord.x;
			unsigned int mouse_y = CanvasSystem::target_height - (unsigned int)mouse_coord.y;
			Entity raycast_target = sCanvas.Raycast(mouse_x, mouse_y);
		//	std::cout << "raycast: " << (unsigned int)raycast_target << std::endl;
			frm->StartSystem("UI Interaction");
			sButton.HandleMouse(*sInputs, raycast_target);
			sSlider.HandleMouse(*sInputs, raycast_target);
			frm->EndSystem("UI Interaction");

			frm->StartSystem("Navigation System");
			sNav.Update(static_cast<float>(frm->getDeltaTime()));
			frm->EndSystem("Navigation System");
		}

		frm->StartSystem("Graphics");
		sRender->Render();
		frm->EndSystem("Graphics");

		frm->StartSystem("Canvas");
		sCanvas.UpdateHierachy();
		sCanvas.DrawOverlay();
		frm->EndSystem("Canvas");

		frm->StartSystem("Particle System");
		if (sScene->mCurrentState == SceneState::PLAY_SCENE)
		{			
			core->GetSystem<ParticleSystemManager>().Update(static_cast<float>(frm->getDeltaTime()));			
		}
		frm->EndSystem("Particle System");

		frm->EndFrame();
		frm->CalculateSystemPercentages();
	}

	void Engine::Draw()
	{
		Core::GetInstance()->GetRenderManager()->Draw();
	}

	void Engine::EndFrame()
	{
		Core::FactoryInstance.UpdateDestroyed();
		Core::GetInstance()->GetSceneSystem()->isSceneUnloaded = true;

		auto window = Core::GetInstance()->GetWindow();
		if (glfwWindowShouldClose(window))
			isRunning = false;
		//auto inputs = Core::GetInstance()->GetInputSystem();
		glfwSwapBuffers(window);
	}

	void Engine::Exit()
	{
		auto& mCanvas = Core::GetInstance()->GetSystem<CanvasSystem>();
		mCanvas.Release();

		Core::GetInstance()->ExitCore();

		SLICE_LOG("Shutting Down Slice Engine.");
	}

}