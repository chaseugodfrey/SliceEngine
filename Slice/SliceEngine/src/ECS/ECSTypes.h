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
#include <variant>
#include "../Physics/CollisionLayer.h"

//#include "PropConfig.h"
//#include <xprop/xproperty.h>

using Entity = entt::entity;
using Registry = entt::registry;

namespace SliceEngine
{
	struct EntityID
	{
		uint64_t value;

		EntityID() : value(0) {}
		EntityID(uint64_t v) : value(v) {}

		operator uint64_t() const { return value; }
	};

	struct SceneGraph
	{
		EntityID entity_id{};

		enum Direction {
			UP = 0,
			DOWN,
			LEFT,
			RIGHT,
			DIRECTIONS
		};

		uint8_t child_count{};

		// rttr doesnt like c style arrays lol
		//uint32_t neighbours[4];
		std::array<Entity, Direction::DIRECTIONS> neighbours{entt::null, entt::null, entt::null, entt::null};
	};

	struct Script
	{
		std::string scriptName;
		//std::unordered_map<std::string, variantVar> scriptableFieldMap;
	};

	struct SliceEntity 
	{
		std::string mName;
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

	struct Transform
	{
		glm::vec3 position{};
		glm::vec3 rotation{};
		glm::vec3 scale{1};

		glm::vec3 previousScale{};

		glm::mat4 transform_local{};
		glm::mat4 transform{};
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
		GUID model;
		GUID texture;
		unsigned char renderTag;
	};

	struct Camera
	{
		int width{}, height{};
		float pov{}, near{}, far{};// Pov is the angle of y of the screen
		GLuint textureID{}, depthTex{};
		unsigned char renderTag{};
	};

	struct Light // TODO: Default 1 directional light for now
	{
		//enum class LightType
		//{
		//	Directional,
		//	Point,
		//	Spot
		//};
		//LightType type = LightType::Directional;
		glm::vec3 color{1.0f, 1.0f, 1.0f};
		float intensity = 1.0f;
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

	};

	struct ColliderShape
	{
		struct BoxData
		{
			JPH::Vec3 scale{ 0.5f, 0.5f,0.5f };
			JPH::Vec3 prevScale{ 0.5f, 0.5f,0.5f };
		};

		struct SphereData
		{
			float radius{ 1.0f };
			float prevRadius{ 1.0f };
		};

		JPH::BodyID bodyID;										// Jolt body reference
		JPH::ObjectLayer layer = Layers::MOVING;									// Collision layer :D
		std::variant<BoxData, SphereData> shapeData = BoxData{};// will add more if we have more shapes :D
		JPH::ShapeRefC shape;									// Jolt shape ref
		JPH::Vec3 offSet{ 0.f,0.f,0.f };						// if we need to offset the collision shape relative to the transform :D
		bool isTrigger = false;									// leaving thjis here in case we need triggers :D

	};

	struct AudioSource
	{
		std::string soundName;
		float currentVolume = 0.3f;
		bool isLoop = false;
		bool isPaused = true;
		bool is3D = true;
		bool playPreview = false;
	};

	struct AudioListener
	{
		FMOD_VECTOR pos{};
	};

	// placeholder particle system component structure for reference
	struct Particle
	{
		bool active{ false };

		float lifetime{};        // how long this particle has left
		float speed{};           // current speed
		float angle{};           // direction (in radians or degrees)

		// optional transform-like data
		glm::vec3 position{};
		glm::vec3 velocity{};    // derived from speed + angle
		glm::vec3 color{};       // if you want per-particle tint

		// flags
		bool has_rigidbody{ false };
	};
	class ParticleSystem
	{
		// ---- System-wide settings ----
		Transform* parent_transform{};     // emitter transform (spawn reference)

		float internal_timer{};            // tracker for emission interval
		float lifetime_timer{};            // system’s overall lifetime
		float duration{};                  // how long the system lasts

		float emission_rate{ 1.f };        // particles/sec
		float emitter_angle_degrees{ 0.0f };
		float emitter_angle_radians{ 0.0f }; // cache
		float arc{ 360.f };                // emission spread

		bool is_repeating{ false };
		bool is_ending{ false };

		uint32_t oldest_particle_index{ 0u };

		float max_particle_lifetime{};
		float particle_speed{};

		glm::vec3 min_random_spawn_pos_relative_to_parent{};
		glm::vec3 max_random_spawn_pos_relative_to_parent{};

		bool fade_over_lifetime{ false };

		uint32_t max_particles{ 50 }; // pool size

		uint32_t internal_current_index{};

		// ---- Particle storage ----
		std::vector<Particle> particles;

	public:
		//ParticleSystem(uint32_t maxCount = 50)
		//	: max_particles(maxCount)
		//{
		//	particles.resize(max_particles);
		//}

		//void Init(Transform* _parent) { parent_transform = _parent; }
		//void Update(float dt);
		//void ResetParticle(uint32_t idx);
		//void DisableParticle(uint32_t idx);
	};
}

#endif