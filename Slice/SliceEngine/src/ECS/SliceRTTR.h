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
		rttr::registration::class_<Transform>(typeid(Transform).name())
			.property("position", &Transform::position)
			.property("rotation", &Transform::rotation)
			.property("scale", &Transform::scale);

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