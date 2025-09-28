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

RTTR_REGISTRATION
{
	rttr::registration::class_<EntityCollide>("EntityCollide")
	.constructor<>()
	.property("firstEntity", &EntityCollide::firstEntity)
	.property("secondEntity", &EntityCollide::secondEntity);
}


#endif