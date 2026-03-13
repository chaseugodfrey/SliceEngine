/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			EventManager.cpp
 author:		Gideon Francis
 email:			g.francis@digipen.edu
 brief:			Handles all events

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include "pch.h"
#include "EventManager.h"
#include "Events.h"

using namespace SliceEngine;

/// <summary>
/// Call all event registering here
/// </summary>
void EventManager::SetupEventManager()
{
	RegisterEvent<ColliderShapeAddedEvent>();
	RegisterEvent<ColliderShapeRemovedEvent>();
	RegisterEvent<RigidBodyAddedEvent>();
	RegisterEvent<RigidBodyRemovedEvent>();
	RegisterEvent<NetworkClientConnectEvent>();
	RegisterEvent<NetworkBindPortEvent>();
	RegisterEvent<GONetworkEvent>();
	//RegisterEvent<SliceEntityModifiedEvent>();
	RegisterEvent<ColliderShapeModifiedEvent>();
	RegisterEvent<RigidBodyModifiedEvent>();
	RegisterEvent<OnSceneLoadedEvent>();
	RegisterEvent<OnSceneStopEvent>();
	RegisterEvent<OnSceneChangeEvent>();

	//Collision Events
	RegisterEvent<OnCollisionEnterEvent>();
	RegisterEvent<OnCollisionStayEvent>();
	RegisterEvent<OnCollisionExitEvent>();
	RegisterEvent<OnTriggerEnterEvent>();
	RegisterEvent<OnTriggerStayEvent>();
	RegisterEvent<OnTriggerExitEvent>();

	//who wrote that stupid commment

	//ui events
	RegisterEvent<OnButtonClickEvent>();
	RegisterEvent<OnButtonHoverEvent>();
	RegisterEvent<OnButtonExitHoverEvent>();
	RegisterEvent<OnButtonReleaseEvent>();
	RegisterEvent<OnSliderValueEvent>();

	RegisterEvent<DebugDrawRayEvent>();
}

void EventManager::Publish(const std::string& eventName, const std::unordered_map<std::string, rttr::variant>& properties)
{
	rttr::type eventType = rttr::type::get_by_name(eventName);

	if (!eventType.is_valid())
	{
		SLICE_LOG_ERROR("Event is not registered with RTTR");
		return;
	}

	auto it = mEventPublishers.find(eventType);
	if (it == mEventPublishers.end())
	{
		SLICE_LOG_ERROR("Event not registered with Event Manager");
		return;
	}

	rttr::variant eventInstance = eventType.create();
	if (!eventInstance.is_valid())
	{
		SLICE_LOG_ERROR("Unable to create event instance");
		return;
	}

	for (const auto& pair : properties)
	{
		rttr::property prop = eventType.get_property(pair.first);
		if (prop.is_valid())
		{
			bool success = prop.set_value(eventInstance, pair.second);
			if (!success)
			{
				SLICE_LOG_ERROR("Unable to set property of event");
			}
		}
		else
		{
			SLICE_LOG_ERROR("Property not found in event");
		}
	}

	it->second(eventInstance);
}

std::vector<std::pair<std::string, std::string>> EventManager::GetEventProperties(const std::string& eventName) const
{
	std::vector<std::pair<std::string, std::string>> properties;
	rttr::type eventType = rttr::type::get_by_name(eventName);

	if (eventType.is_valid())
	{
		for (const auto& prop : eventType.get_properties())
		{
			properties.emplace_back(prop.get_name().to_string(), prop.get_type().get_name().to_string());
		}
	}

	return properties;
}

std::vector<std::string> EventManager::GetRegisteredEvents() const
{
	std::vector<std::string> names;
	for (const auto& type : mRegisteredEventTypes)
	{
		names.push_back(type.get_name().to_string());
	}

	return names;
}