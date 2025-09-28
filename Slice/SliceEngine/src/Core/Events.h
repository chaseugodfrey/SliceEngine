#ifndef EVENTS_H
#define EVENTS_H
#include "ECS/ECSTypes.h"
/*
	Define any events here
*/

struct EntityCollide
{
	Entity firstEntity;
	Entity secondEntity;
};

struct ColliderShapeAddedEvent {
	Entity entity;
};

struct ColliderShapeRemovedEvent {
	Entity entity;
};

struct RigidBodyAddedEvent {
	Entity entity;
};

struct RigidBodyRemovedEvent {
	Entity entity;
};

RTTR_REGISTRATION
{
	rttr::registration::class_<EntityCollide>("EntityCollide")
	.constructor<>()
	.property("firstEntity", &EntityCollide::firstEntity)
	.property("secondEntity", &EntityCollide::secondEntity);

	rttr::registration::class_<ColliderShapeAddedEvent>("ColliderShapeAdded")
	.constructor<>()
	.property("entity", &ColliderShapeAddedEvent::entity);

	rttr::registration::class_<ColliderShapeRemovedEvent>("ColliderShapeRemoved")
	.constructor<>()
	.property("entity", &ColliderShapeRemovedEvent::entity);

	rttr::registration::class_<RigidBodyAddedEvent>("RigidBodyAdded")
	.constructor<>()
	.property("entity", &RigidBodyAddedEvent::entity);

	rttr::registration::class_<RigidBodyRemovedEvent>("RigidBodyRemoved")
	.constructor<>()
	.property("entity", &RigidBodyRemovedEvent::entity);
}


#endif