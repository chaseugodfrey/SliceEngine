#ifndef ECS_TYPES
#define ECS_TYPES

#include <entt.hpp>
#include <vec3.hpp>
#include <mat4x4.hpp>
#include <gtc/quaternion.hpp>
#include <glfw3.h>
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

	struct testStruct
	{
		int val;
	};

	struct Transform
	{
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;

		//Transform()
		//{
		//	// need change to our own vec3
		//	position = glm::vec3(0, 0, 0);
		//	rotation = glm::vec3(0, 0, 0);
		//	scale = glm::vec3(0, 0, 0);

		//}
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
		float pov, near, far;
		GLuint textureID{}, depthTex{};
	};

	struct RigidBody
	{
		bool isKinematic{};
	};



}

#endif