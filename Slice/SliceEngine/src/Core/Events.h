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

struct NetworkClientConnectEvent {
	//Entity entity;
	std::string ip;
	std::string port;
}; 
struct NetworkBindPortEvent {
	std::string port;
};

struct GONetworkEvent {
	Entity entity;
	bool create;
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

struct OnParent
{
	Entity parent;
	Entity child;
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

	rttr::registration::class_<NetworkClientConnectEvent>("NetworkClientAdded")
	.constructor<>()
	.property("ip", &NetworkClientConnectEvent::ip)
	.property("port", &NetworkClientConnectEvent::port);

	rttr::registration::class_<NetworkBindPortEvent>("NetworkPortBinded")
	.constructor<>()
	.property("port", &NetworkBindPortEvent::port);

	rttr::registration::class_<GONetworkEvent>("GONetworked")
	.constructor<>()
	.property("entity", &GONetworkEvent::entity)
	.property("create", &GONetworkEvent::create);
}


#endif