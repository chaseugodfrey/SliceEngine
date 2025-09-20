#ifndef ECS_TYPES
#define ECS_TYPES

#include <entt.hpp>
#include <gtc/quaternion.hpp>
#include <glfw3.h>
#include <variant>
//#include "PropConfig.h"
//#include <xprop/xproperty.h>

using Entity = entt::entity;
using Registry = entt::registry;

namespace SliceEngine
{
	struct SliceEntity 
	{
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

		glm::mat4 transform{};
	};

	struct UITransform
	{
		// blank for now because I just need to use this for factory stuff
	};

	//XPROPERTY_REG(Transform);

	struct Renderer
	{
		// May need to change if rendering pipeline is diff
		std::string model;
		std::string texture;
	};

	struct Camera
	{
		int width, height;
		float pov, near, far;// Pov is the angle of y of the screen
		GLuint textureID{}, depthTex{};
	};

	struct RigidBody
	{
		JPH::BodyID bodyID;								   // Jolt body reference
		JPH::EMotionType motionType;					   // Static/Kinematic/Dynamic
		JPH::ObjectLayer layer;							   // Collision layer
		bool isActive = true;

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
			JPH::Vec3 halfExtend{ 1.0f,1.0f,1.0f };
		};

		struct SphereData
		{
			float radius{ 1.0f };
		};

		ColliderType type = ColliderType::Box;					// Set Box Collider as default
		std::variant<BoxData, SphereData> shapeData = BoxData{};// will add more if we have more shapes
		JPH::ShapeRefC shape;									// Jolt shape ref
		JPH::Vec3 offSet{ 0.f,0.f,0.f };						// if we need to offset the collision shape relative to the transform
		bool isTrigger = false;									// leaving thjis here in case we need trniggers

	};





}

#endif