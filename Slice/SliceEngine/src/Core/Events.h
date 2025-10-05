/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Events.h
 author:		Gideon Francis
 email:			g.francis@digipen.edu
 brief:			Events that can happen

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
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

RTTR_REGISTRATION
{
	rttr::registration::class_<EntityCollide>("EntityCollide")
	.constructor<>()
	.property("firstEntity", &EntityCollide::firstEntity)
	.property("secondEntity", &EntityCollide::secondEntity);

}


#endif