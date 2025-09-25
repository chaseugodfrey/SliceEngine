#ifndef SLICE_RTTR_H
#define SLICE_RTTR_H

#include "../Graphics/WorldSpaceGraphicsSystem.h"
#include "../Systems/TransformSystem.h"
#include "../Systems/SoundSystem.h"

#include <rttr/registration.h>
namespace SliceEngine
{
	RTTR_REGISTRATION
	{
		// Register the interface for base system
		//rttr::registration::class_<IBaseSystem>("IBaseSystem")
		//.method("Bind", &IBaseSystem::Bind)
		//.method("Unbind", &IBaseSystem::Unbind);
		//rttr::registration::class_<WorldSpaceGraphicsSystem>("WorldSpaceGraphicsSystem")
		//	.constructor<>()
		//	.method("EntityOnEnter", &WorldSpaceGraphicsSystem::EntityOnEnter)
		//	.method("EntityOnExit", &WorldSpaceGraphicsSystem::EntityOnExit)
		//	.method("EntityOnUpdate", &WorldSpaceGraphicsSystem::EntityOnUpdate);
		//rttr::registration::class_<TransformSystem>("TransformSystem")
		//	.constructor<>()
		//	.method("EntityOnEnter", &TransformSystem::EntityOnEnter)
		//	.method("EntityOnExit", &TransformSystem::EntityOnExit)
		//	.method("EntityOnUpdate", &TransformSystem::EntityOnUpdate);
		//rttr::registration::class_<SoundSystem>("SoundSystem")
		//	.constructor<>()
		//	.method("EntityOnEnter", &SoundSystem::EntityOnEnter)
		//	.method("EntityOnExit", &SoundSystem::EntityOnExit)
		//	.method("EntityOnUpdate", &SoundSystem::EntityOnUpdate);

	rttr::registration::class_<std::vector<uint32_t>>("VectorUInt32")
		.constructor<>()
		.method("size", &std::vector<uint32_t>::size)
		.method("at", static_cast<uint32_t& (std::vector<uint32_t>::*)(size_t)>(&std::vector<uint32_t>::at))
		.method("push_back", static_cast<void (std::vector<uint32_t>::*)(const uint32_t&)>(&std::vector<uint32_t>::push_back))
		.method("push_back", static_cast<void (std::vector<uint32_t>::*)(uint32_t&&)>(&std::vector<uint32_t>::push_back));

	rttr::registration::class_<std::array<uint32_t, 4>>("Array4UInt32")
		.constructor<>()
		.method("size", &std::array<uint32_t, 4>::size)
		.method("at", static_cast<uint32_t& (std::array<uint32_t, 4>::*)(size_t)>(&std::array<uint32_t, 4>::at))
		.method("fill", &std::array<uint32_t, 4>::fill)
		.method("front", static_cast<uint32_t& (std::array<uint32_t, 4>::*)()>(&std::array<uint32_t, 4>::front))
		.method("back", static_cast<uint32_t& (std::array<uint32_t, 4>::*)()>(&std::array<uint32_t, 4>::back));
	
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


	rttr::registration::class_<Transform>(typeid(Transform).name())
		.constructor<>()
		.property("position", &Transform::position)
		.property("rotation", &Transform::rotation)
		.property("scale", &Transform::scale);

	rttr::registration::class_<SceneGraph>(typeid(SceneGraph).name())
		.constructor<>()
		.property("neighbours", &SceneGraph::neighbours);

	rttr::registration::class_<SliceEntity>(typeid(SliceEntity).name())
		.constructor<>()
		.property("mName", &SliceEntity::mName);
        		rttr::registration::class_<RigidBody>(typeid(RigidBody).name())
			.property("Motiontype", &RigidBody::motionType)
			.property("layer", &RigidBody::layer)
			.property("isActive", &RigidBody::isActive)
			.property("mass", &RigidBody::mass)
			.property("friction", &RigidBody::friction)
			.property("restituition", &RigidBody::restitution)
			.property("linearDamping", &RigidBody::linearDamping)
			.property("angularDamping", &RigidBody::angularDamping);

		rttr::registration::class_<ColliderShape>(typeid(ColliderShape).name())
			.property("ColliderType", &ColliderShape::type)
			.property("ShapeData", &ColliderShape::shapeData)
			.property("offSet", &ColliderShape::offSet)
			.property("isTrigger", &ColliderShape::isTrigger);
	}

}
#endif