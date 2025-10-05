/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			SliceRTTR.cpp
 author:		Gideon Francis
 email:			g.francis@digipen.edu
 brief:			RTTR

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef SLICE_RTTR_H
#define SLICE_RTTR_H

#include "../Graphics/WorldSpaceGraphicsSystem.h"
#include "../Systems/TransformSystem.h"
#include "../Systems/SoundSystem.h"

#include <rttr/registration.h>
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
	rttr::registration::class_<std::vector<uint32_t>>("VectorUInt32")
		.constructor<>()
		.method("size", &std::vector<uint32_t>::size)
		.method("at", static_cast<uint32_t & (std::vector<uint32_t>::*)(size_t)>(&std::vector<uint32_t>::at))
		.method("push_back", static_cast<void (std::vector<uint32_t>::*)(const uint32_t&)>(&std::vector<uint32_t>::push_back))
		.method("push_back", static_cast<void (std::vector<uint32_t>::*)(uint32_t&&)>(&std::vector<uint32_t>::push_back));

	register_std_array<uint32_t, 4>("Array4UInt32");
	register_std_array<Entity, 4>("Array4Entity");

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
		.property("entity_id", &SceneGraph::entity_id)
		.property("neighbours", &SceneGraph::neighbours);

	rttr::registration::class_<EntityID>("EntityID")
		.constructor<>()(rttr::policy::ctor::as_object)
		.constructor<uint32_t>()
		.property("value", &EntityID::value);

	rttr::registration::class_<SliceEntity>(typeid(SliceEntity).name())
		.constructor<>()
		.property("mName", &SliceEntity::mName);
	rttr::registration::class_<RigidBody>(typeid(RigidBody).name())
		.property("isKinematic", &RigidBody::isKinematic)
		.property("gravityFactor", &RigidBody::gravityFactor)
		.property("CollisionDetection", &RigidBody::CollisionDetection)
		.property("mass", &RigidBody::mass)
		.property("friction", &RigidBody::friction)
		.property("restituition", &RigidBody::restitution)
		.property("linearDamping", &RigidBody::linearDamping)
		.property("angularDamping", &RigidBody::angularDamping);

	rttr::registration::class_<ColliderShape>(typeid(ColliderShape).name())
		.property("layer", &ColliderShape::layer)
		.property("ShapeData", &ColliderShape::shapeData)
		.property("offSet", &ColliderShape::offSet)
		.property("isTrigger", &ColliderShape::isTrigger);

	rttr::registration::class_<GUID>("GUID")
		.constructor<>()
		.constructor<uint64_t>()
		.property_readonly("Value", &GUID::GetGUID);
	}
}
#endif