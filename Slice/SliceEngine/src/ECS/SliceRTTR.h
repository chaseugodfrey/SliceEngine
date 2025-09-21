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

	rttr::registration::class_<std::vector<uint64_t>>("VectorUInt64")
		.constructor<>()
		.method("size", &std::vector<uint64_t>::size)
		.method("at", static_cast<uint64_t & (std::vector<uint64_t>::*)(size_t)>(&std::vector<uint64_t>::at))
		.method("push_back", static_cast<void (std::vector<uint64_t>::*)(const uint64_t&)>(&std::vector<uint64_t>::push_back))
		.method("push_back", static_cast<void (std::vector<uint64_t>::*)(uint64_t&&)>(&std::vector<uint64_t>::push_back));

	rttr::registration::class_<Transform>(typeid(Transform).name())
		.property("position", &Transform::position)
		.property("rotation", &Transform::rotation)
		.property("scale", &Transform::scale);

	rttr::registration::class_<SceneView>(typeid(SceneView).name())
		.property("parent", &SceneView::parentGUID)
		.property("children", &SceneView::childrenGUID)
		.property("up", &SceneView::upGUID)
		.property("down", &SceneView::downGUID);
	}

}
#endif