#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H
#include "Singleton.h"
#include <entt.hpp>

class EventManager : public Singleton<EventManager>
{
private:
	entt::dispatcher mDispatcher{};
	std::vector<rttr::type> mRegisteredEventTypes;
	std::unordered_map<rttr::type, std::function<void(rttr::variant&)>> mEventPublishers;

public:
	EventManager() = default;
	~EventManager() = default;

	// dont allow copying
	EventManager(const EventManager&) = delete;
	EventManager& operator=(const EventManager&) = delete;

	/// <summary>
	/// Register the event type to allow for runtime reeflection.
	/// Useful for editor function calls
	/// </summary>
	/// <typeparam name="Event"></typeparam>
	template<typename Event>
	void RegisterEvent()
	{
		rttr::type eventType = rttr::type::get<Event>();
		if (!eventType.is_valid())
		{
			SLICE_LOG_ERROR("Event not registered with RTTR");
			return;
		}

		// If this type is already registered then dont register again
		if (mEventPublishers.count(eventType))
		{
			return;
		}

		mRegisteredEventTypes.push_back(eventType);

		mEventPublishers[eventType] = [this](rttr::variant& eventInstance)
		{
			// convert the variant to the event for dispatcher to trigger
			if (eventInstance.convert<Event>())
			{
				mDispatcher.trigger(eventInstance.get_value<Event>());
			}
			else
			{
				SLICE_LOG_ERROR("Unable to convert rttr::variant to event");

			}
		};
	}

	template<typename Event, auto Candidate, typename Instance>
	void Subscribe(Instance* instance)
	{
		mDispatcher.sink<Event>().connect<Candidate>(*instance);
	}

	template <typename Event, typename... Args>
	void Publish(Args&&... args)
	{
		mDispatcher.trigger<Event>(std::forward<Args>(args)...);
	}

	//void PublishByName
};

#endif