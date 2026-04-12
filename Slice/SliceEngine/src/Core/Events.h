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


namespace SliceEngine
{
	using Entity = entt::entity;
	using Registry = entt::registry;

	/*
		Define any events here
	*/

	struct DebugDrawRayEvent
	{
		glm::vec3 Origin;
		glm::vec3 Dir;
		float magnitude;
	};

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

	struct AnimationEvent {
		std::string funcName;
		std::string scriptName;
		Entity entity;
	};

	//struct SliceEntityModifiedEvent
	//{
	//	Entity entity;
	//};

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
		std::filesystem::path scenePath;
		bool isSceneLoaded;
		std::string navMeshBinPath;
	};

	struct OnSceneStopEvent
	{
		bool isSceneStopped;
	};

	struct OnParent
	{
		Entity parent;
		Entity child;
	};


	// Physics collision events
	struct OnCollisionEnterEvent
	{
		Entity entity;
		Entity other;
	};
	struct OnCollisionStayEvent
	{
		Entity entity;
		Entity other;
	};
	struct OnCollisionExitEvent
	{
		Entity entity;
		Entity other;
	};
	struct OnTriggerEnterEvent
	{
		Entity entity;
		Entity other;
	};
	struct OnTriggerStayEvent
	{
		Entity entity;
		Entity other;
	};
	struct OnTriggerExitEvent
	{
		Entity entity;
		Entity other;
	};

	struct OnButtonClickEvent
	{
		Entity entity;
	};

	struct OnButtonHoverEvent
	{
		Entity entity;
	};

	struct OnButtonExitHoverEvent
	{
		Entity entity;
	};

	struct OnButtonReleaseEvent
	{
		Entity entity;
	};
	struct OnSliderValueEvent
	{
		Entity entity;
		float value;
	};

	struct OnSpriteAnimStopEvent
	{
		Entity entity;
	};

	struct OnSpriteAnimLoopEvent
	{
		Entity entity;
	};

	struct OnSceneChangeEvent
	{

	};

	struct OnPrefabModifiedEvent
	{
		OnPrefabModifiedEvent(Entity ent, SliceEngine::GUID g_uid) : entity(ent), guid(g_uid)
		{

		}
		Entity entity;
		SliceEngine::GUID guid;
	};

	struct OnPrefabDeletedEvent
	{
		OnPrefabDeletedEvent(Entity ent, SliceEngine::GUID g_uid) : entity(ent), guid(g_uid)
		{

		}
		Entity entity;
		SliceEngine::GUID guid;
	};

	struct OnPrefabSerializedEvent
	{
		OnPrefabSerializedEvent(Entity ent, SliceEngine::GUID g_uid) : entity(ent), guid(g_uid)
		{

		}
		Entity entity;
		SliceEngine::GUID guid;
	};

	struct AssetExistEvent
	{
		AssetExistEvent(std::string name) : assetName(name)
		{

		}

		std::string assetName;
	};

	struct OnPlayEvent
	{

	};

	struct OnStopEvent
	{

	};

	struct OnGameStopEvent
	{

	};

	// lol this is stupid but whatever
}


#endif