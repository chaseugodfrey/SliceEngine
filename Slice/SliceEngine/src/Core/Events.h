#ifndef EVENTS_H
#define EVENTS_H
#include "ECS/ECSTypes.h"
#include <rttr/registration.h>
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

struct ColliderShapeModifiedEvent
{
	Entity entity;
};

struct RigidBodyModifiedEvent
{
	Entity entity;
};

struct OnSceneLoadedEvent
{
	bool isSceneLoaded;
};

RTTR_REGISTRATION
{
	rttr::registration::class_<EntityCollide>("EntityCollide")
	.constructor<>()
	.property("firstEntity", &EntityCollide::firstEntity)
	.property("secondEntity", &EntityCollide::secondEntity);

}


#endif