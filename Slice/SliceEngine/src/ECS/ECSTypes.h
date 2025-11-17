/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			ECSTypes.h
 author:		
 email:			
 brief:			Structs for the different components

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef ECS_TYPES
#define ECS_TYPES

#include <entt.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glfw3.h>
#include <../fmod/include/fmod.hpp>
#include <variant>
#include "../Physics/CollisionLayer.h"
#include <rttr/rttr_enable.h>
#include "Resource/Resource.h"
#include "Resource/ResourceManager.h"
#include "Animator/FSMSystem.h"
#include "Resource/Skeleton.h"

//#include "PropConfig.h"
//#include <xprop/xproperty.h>

using Entity = entt::entity;
using Registry = entt::registry;

namespace SliceEngine
{
	struct SliceEntity 
	{
		std::string mName;
		std::string mTag{ "default" };
		bool active;

		SliceEntity() : active(true) {}
	};

	struct EngineEntity
	{
		bool active;
		
		EngineEntity() : active(true) {}
	};

	struct testStruct
	{
		int val;
	};

	struct SceneGraph
	{
		uint32_t entity_id{};

		enum Direction {
			UP = 0,
			DOWN,
			LEFT,
			RIGHT,
			DIRECTIONS
		};

		std::array<Entity, Direction::DIRECTIONS> neighbours{ entt::null, entt::null, entt::null, entt::null };

		RTTR_ENABLE();
	};

	struct Script
	{
		std::string scriptName;

		// purely for serialization and deserialization
		// to save scriptable field values in scenes and for prefabs(?)
		std::unordered_map<std::string, rttr::variant> scriptableFieldMap;

		RTTR_ENABLE();
	};

    struct Transform
    {

        glm::vec3 position{ 0.0f, 0.0f, 0.0f };
        glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
        glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
        glm::mat4 transform_local{ 1.0f };
        glm::mat4 transform{ 1.0f };

		glm::vec3 eulerAnglesHint{ 0.0f, 0.0f, 0.0f };

		uint32_t collisionMask;

		RTTR_ENABLE();
    };

	struct UITransform
	{
		// blank for now because I just need to use this for factory stuff
	};

	//XPROPERTY_REG(Transform);

	enum RENDER_TAG : unsigned char
	{
		DEBUG_OBJ_TAG		= 0x01,
		DEBUG_FRUSTRUM_TAG	= 0x02,
		DEBUG_GRID_TAG		= 0x04
	};

	struct Renderer
	{
		// May need to change if rendering pipeline is diff
		//GUID model = (GUID)Type<SliceEngineTypes::Model>::defaultResourceGUID; 
		//GUID material = (GUID)Type<SliceEngineTypes::Material>::defaultResourceGUID;

		Handle<SliceEngineTypes::Model> modelHandle;
		Handle<SliceEngineTypes::Material> materialHandle;

		unsigned char meshOffset{ 0 };
		unsigned char renderTag{};
		bool skinned{ false };

		RTTR_ENABLE();
	};

	struct Camera
	{
		int width{}, height{};
		float pov{}, near{}, far{};// Pov is the angle of y of the screen
		GLuint textureID{}, depthTex{};
		unsigned char renderTag{};

		RTTR_ENABLE();
	};

	struct Light // TODO: Default 1 directional light for now
	{
		enum LightType : unsigned char
		{
			Light_Directional = 0
			,Light_Point
			,Light_Spot
		};
		glm::vec3 color{1.0f, 1.0f, 1.0f};
		float intensity{ 1.0f };
		GLuint depthTex{};
		GLuint shadowCubeMap{};
		LightType type = LightType::Light_Point;

		RTTR_ENABLE();
	};

	struct Prefab
	{
		// GUID reference to original prefab
		GUID prefabGUID;

		Handle<SliceEngineTypes::Prefab> prefabHandle;

		RTTR_ENABLE();
	};

	struct RigidBody
	{
		
		bool isKinematic = false;		// Set to Kinematic :D
		float gravityFactor = 1.0f;		// Gravity multiplier:D
		JPH::EMotionQuality CollisionDetection = JPH::EMotionQuality::Discrete; // Motion quality(Discrete or Continuous) :D

		// Physics properties
		float mass = 1.0f; //:D
		float friction = 0.5f;
		float restitution = 0.0f;						   // Bounciness
		float linearDamping = 0.05f;    //:D
		float angularDamping = 0.05f;	//:D


		//To add in Inspector
		struct FreezeOptions
		{
			bool freezeX = false;
			bool freezeY = false;
			bool freezeZ = false;
		};

		FreezeOptions freezePosition;
		FreezeOptions freezeRotation;

		RTTR_ENABLE();
	};

	struct ColliderShape
	{
		struct BoxData
		{
			JPH::Vec3 scale{ 0.5f, 0.5f,0.5f };
		};

		struct SphereData
		{
			float radius{ 0.5f };
		};

		struct CapsuleData
		{
			float radius{ 0.5f };
			float height{ 0.5f };
		};

		JPH::BodyID bodyID;										// Jolt body reference
		JPH::ObjectLayer layer = Layers::MOVING;									// Collision layer :D
		std::variant<BoxData, SphereData, CapsuleData> shapeData = BoxData{};// will add more if we have more shapes :D
		JPH::ShapeRefC shape;									// Jolt shape ref
		JPH::Vec3 offSet{ 0.f,0.f,0.f };						// if we need to offset the collision shape relative to the transform :D
		bool isTrigger = false;									// leaving thjis here in case we need triggers :D

		ColliderShape() = default;
		ColliderShape(BoxData data) : shapeData(data) {};
		ColliderShape(SphereData data) : shapeData(data) {};
		ColliderShape(CapsuleData data) : shapeData(data) {};

	private:
		inline static const BoxData defaultBoxData{};
		inline static const SphereData defaultSphereData{};
		inline static const CapsuleData defaultCapsuleData{};		
	public:
		// Getters
		const BoxData& GetBoxData() const {
			return std::holds_alternative<BoxData>(shapeData) ?
				std::get<BoxData>(shapeData) : defaultBoxData;
		}

		const SphereData& GetSphereData() const {
			return std::holds_alternative<SphereData>(shapeData) ?
				std::get<SphereData>(shapeData) : defaultSphereData;
		}

		const CapsuleData& GetCapsuleData() const {
			return std::holds_alternative<CapsuleData>(shapeData) ?
				std::get<CapsuleData>(shapeData) : defaultCapsuleData;
		}

		// Setters
		void SetBoxData(const BoxData& data) { shapeData = data; }
		void SetSphereData(const SphereData& data) { shapeData = data; }
		void SetCapsuleData(const CapsuleData& data) { shapeData = data; }

		RTTR_ENABLE();
	};

	struct AudioSource
	{
		enum VolumeRollOff : int
		{
			Logarithmic = 0,
			Linear = 1
		};
		//std::string soundName;
		//GUID soundGUID = (GUID)9244272128099795086;
		std::vector<GUID> soundGUIDs = { (GUID)9244272128099795086 };
		FMOD::Channel* channel = nullptr;
		FMOD::Channel* previewChannel = nullptr;
		int priority = 128;
		bool isMute = false;
		bool isLoop = false;
		bool isPaused = true;
		float currentVolume = 0.3f;
		float pitch = 1.0f;
		float stereoPan = 0.0f;
		float spatialBlend = 1.0f;
		//3D effects
		float dopplerLevel = 1.0f;
		float spread = 1.0f;
		VolumeRollOff volumeRollOff = Logarithmic;
		float minDistance = 1.0f;
		float maxDistance = 500.0f;

		float minInterval = 0.0f;
		bool playOnAwake = false;

		bool _playTrigger = false;
		bool playPreview = false;

		RTTR_ENABLE();
	};

	struct AudioListener
	{
		glm::vec3 listenerPos{};
	};

	// placeholder particle system component structure for reference
	struct Particle
	{
		bool active{ false };
		float age{};             // how long this particle has been alive
		
		glm::vec3 finalPosition{};	// including parent transform position if localspace
		glm::vec3 position{};
		glm::quat rotation{};
		glm::vec3 scale{};
		glm::vec3 velocity{};    // derived from speed + angle
		glm::vec4 colour{};       // if you want per-particle tint
	};

	struct ParticleRenderPart
	{
		glm::mat4 transform{}; // has position, rotation, scale calculated
		glm::vec4 colour{};
		GLuint textureID{};

	};
	struct ParticleSystem
	{
		enum ValueType
		{
			CONSTANT,
			CURVE,
			TWO_CONSTANTS
		};

		Transform* parentTransform{ nullptr };

		// System Settings
		float duration{};                       // how long the system should last, 0.0f = forever
		float speed{};							// to add
		bool isRepeating{ false };
		bool isLocalSpace{ false };				// false means world space
		// Lifetime
		bool hasRandomParticleLifetime{ false };	// can remove

		ValueType initialLifetimeType{ CONSTANT };
		float lifetime{};
		float minParticleLifetime{};
		float maxParticleLifetime{};
		// Rotation
		bool hasRandomInitialRotation{ false };		// can remove
		bool isInitialRotation3D{ false };			// to add
		ValueType initialRotationType{ CONSTANT };	// to add
		glm::quat rotation{};
		glm::quat minRandomRotation{};
		glm::quat maxRandomRotation{};
		glm::vec3 eulerHint{};
		glm::vec3 minEulerHint{};
		glm::vec3 maxEulerHint{};
		
		inline void Set1DRotation(float val)
		{
			eulerHint.x = val;
		}

		inline float Get1DRotation()
		{
			return eulerHint.x;
		}

		// Size/Scale
		ValueType scaleType{ CONSTANT };
		glm::vec3 scale{ 1.0f };
		glm::vec3 minRandomScale{ 1.0f };
		glm::vec3 maxRandomScale{ 1.0f };
		bool destroyOnExpire{ false };
		bool hasRandomScale{ false };				// can remove
		uint64_t maxParticles{ 1000 };            // pool size. default 200

		bool hasGravity{ false };					// can remove
		float gForce{0.0f};

		// EMISSION
		float emissionRate{ 0.0f };              // particles/sec
		// Bursts		
		struct Burst
		{
			uint64_t numParticles{};
			uint64_t burstRepetitions{};		// how many times to do the burst
			float burstPeriod{};				// how far apart in time should each repetition be
			float triggerTime{};				// if greater than systemTimer, trigger burst
			bool triggered{ false };

			uint64_t repsDone{};
			float repTimer{};
		};
		std::vector<Burst> bursts{}; 

		bool hasBursts{ false };				// can remove
		uint64_t numBursts{};					// can remove


		// Shape Settings
		enum ShapeType
		{
			CONE,
			SPHERE,
			BOX,
			EDGE,
			CIRCLE,
			RECTANGLE
		} shapeType;

		float coneAngle{};
		float shapeRadius{};					// to add
		float shapeArc{};						// to add

		glm::vec3 axis = glm::vec3(0, 0, 0);   // emission spread - can be internal
		// Initial Position
		bool hasRandomSpawnPos{ false };        // can be calculated - can be internal
		glm::vec3 minRandomSpawnPos{};
		glm::vec3 maxRandomSpawnPos{};

		// Color
		bool hasRandomColour{ false };				// can remove
		ValueType colorValueType{ CONSTANT };		// to add
		glm::vec4 colour{ 0.0f, 0.0f, 0.0f, 1.0f };
		glm::vec4 minRandomColour{ 0.0f, 0.0f, 0.0f, 1.0f };
		glm::vec4 maxRandomColour{ 0.0f, 0.0f, 0.0f, 1.0f };
		bool colorOverLifetime{ false };			// to add
		std::map<float, glm::vec4> colorLifeTimeMap;	// to add

		bool hasRandomVelocity{ false };			// can remove
		glm::vec3 velocity{ 1.0f };
		glm::vec3 minRandomVelocity{ 1.0f };
		glm::vec3 maxRandomVelocity{ 1.0f };

		bool fadeOverLifetime{ false };				// can remove
		bool hasCollision{ false };

		// Renderer
		GLuint textureID;							// change to guid
		enum RenderMode
		{
			BILLBOARD,
			MESH
		} renderMode;
		GUID textureGUID;
		GUID materialGUID;
		GUID meshGUID;
		Handle<SliceEngineTypes::Texture> textureHandle;
		Handle<SliceEngineTypes::Material> materialHandle;
		Handle<SliceEngineTypes::Mesh> meshHandle;

		// ------- Internal ----------
		std::vector<Particle> particles{};
		uint64_t awaitingIndex{};				// index that is waiting for ActivateParticle
		uint64_t oldestIndex{};					// oldest particle index as backup when exceeding maxParticles, use this particle then +1 the index

		// Main Particle Storage Poooool

		bool systemEnding{ false };				// Turns true when particle system expired and just waiting for its particles to all expire
		bool expired{ false };					// Turns true when all particles have expired + systemEnding is true
		bool isActive{ true };
		float systemTimer{};					// system's overall lifetime

		float emissionAccumulator{};
	};

	struct Timeline
	{
		int32_t f_current{}, f_min{ 0 }, f_max{ 60 };
		bool isPlaying{}, isLoop{};
	};

	struct Animator
	{

		Handle<SliceEngineTypes::StateMachine> Handle_stateMachine;
		FSMSystem stateMachine;
		//FSMSystem stateMachine;

		float current_time{};
		Timeline timeline;

		bool is_bone{ true };

		std::vector<glm::mat4> final_tforms;
		std::bitset<MAX_BONES> inverse_flags{};
		std::unordered_map<unsigned int, glm::mat4> inverse_map{};

		Handle<SliceEngineTypes::AnimationPackage> Handle_curr_anim_pkg;
		Handle<SliceEngineTypes::Skeleton> Handle_skeleton;

		SliceEngineTypes::AnimationPackage curr_anim_pkg;


		//tbh these 2 set_x stuff shld be taking in a guid/handle to these resources, then creating and instance of it

		/*
		void SetAnimationPackage(SliceEngine::GUID anim)
		{
			curr_anim_pkg.mGUID = anim;
		}
		void SetSkeleton(SliceEngineTypes::Skeleton* skele) 
		{
			skeleton = skele;
		}
		*/


		void SetInverseRoots() {
			for (unsigned int i{}; i < inverse_flags.size(); ++i) {
				if (inverse_flags[i]) {
					inverse_map[i] = glm::inverse(final_tforms[i]);
				}
			}
		}
		void SetInverseRoot(unsigned int idx) 
		{
			assert(idx < final_tforms.size());
			inverse_map[idx] = glm::inverse(final_tforms[idx]);
		}

		void PlayAnimation(unsigned int idx) 
		{
			stateMachine.EFSM.currState->curr_anim_idx = idx;
		}

		std::vector<glm::mat4> const& GetFinalTform() const
		{
			return final_tforms;
		}

		RTTR_ENABLE();
	};

	struct Bone 
	{
		Entity skeleton_root{};
		unsigned int frame_idx{};

		RTTR_ENABLE();
	};

	
}

#endif