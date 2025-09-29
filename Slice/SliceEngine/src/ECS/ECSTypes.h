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
	struct SceneGraph
	{
		uint32_t entity_id;

		enum Direction {
			UP = 0,
			DOWN,
			LEFT,
			RIGHT,
			DIRECTIONS
		};
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
		glm::vec3 scale{};

		glm::vec3 previousScale{};

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
		std::string model;
		std::string texture;
		unsigned char renderTag;
	};

	struct Camera
	{
		int width, height;
		float pov, near, far;// Pov is the angle of y of the screen
		GLuint textureID{}, depthTex{};
		unsigned char renderTag;
	};

	struct RigidBody
	{
		
		bool isKinematic = false;		// Set to Kinematic
		float gravityFactor = 1.0f;		// Gravity multiplier
		JPH::EMotionQuality CollisionDetection = JPH::EMotionQuality::Discrete; // Motion quality(Discrete or Continuous)

		// Physics properties
		float mass = 1.0f;
		float friction = 0.5f;
		float restitution = 0.0f;						   // Bounciness
		float linearDamping = 0.05f;    
		float angularDamping = 0.05f;

	};

	struct ColliderShape
	{
		enum class ColliderType
		{
			Box,
			Sphere
		};

		struct BoxData
		{
			JPH::Vec3 scale{ 0.5f, 0.5f,0.5f };
		};

		struct SphereData
		{
			float radius{ 1.0f };
		};

		JPH::BodyID bodyID;										// Jolt body reference
		JPH::ObjectLayer layer = Layers::MOVING;									// Collision layer
		ColliderType type = ColliderType::Box;					// Set Box Collider as default
		std::variant<BoxData, SphereData> shapeData = BoxData{};// will add more if we have more shapes
		JPH::ShapeRefC shape;									// Jolt shape ref
		JPH::Vec3 offSet{ 0.f,0.f,0.f };						// if we need to offset the collision shape relative to the transform
		bool isTrigger = false;									// leaving thjis here in case we need trniggers

	};

	struct AudioSource
	{
		std::string soundName;
		float currentVolume;
		bool isLoop;
		bool isPaused;
		bool is3D;
	};



}

#endif