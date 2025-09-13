#ifndef ECS_TYPES
#define ECS_TYPES

#include <entt.hpp>
#include <vec3.hpp>
#include <mat4x4.hpp>
#include <gtc/quaternion.hpp>
#include <glfw3.h>
//#include <xprop/xproperty.h>
//#include <rttr/registration.h>

using Entity = entt::entity;
using Registry = entt::registry;

namespace SliceEngine
{
	struct testStruct
	{
		int val;
	};

	struct Transform
	{
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;
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
		float pov, near, far;
		GLuint textureID{}, depthTex{};
	};

	struct RigidBody
	{
		bool isKinematic{};
	};


	//RTTR_REGISTRATION
	//{
	//rttr::registration::class_<Transform>("Transform")
	//	.property("position", &Transform::position)
	//	.property("rotation", &Transform::rotation)
	//	.property("scale", &Transform::scale);
	//rttr::registration::class_<Renderer>("Renderer")
	//	.property("model", &Renderer::model)
	//	.property("texture", &Renderer::texture);
	//}
}

#endif