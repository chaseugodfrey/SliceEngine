#ifndef BASE_SYSTEM_H
#define BASE_SYSTEM_H

#include <entt.hpp>
#include "ECSTypes.h"
namespace SliceEngine
{
	class IBaseSystem
	{
	public:
		virtual ~IBaseSystem() = default;
		virtual void Bind(Registry& reg) = 0;
		virtual void Unbind() = 0;
		virtual void Update(float dt) = 0;
	};

	/// <summary>
	/// System Tag - Used to keep track of entities within the system
	/// ... Required - All components that is used in this system
	/// Example : PhysicSystem : BaseSystem<PhysicEntity, Transform, RigidBody> 
	/// 
	/// Only EntityOnEnter, EntityOnExit and EntityOnUpdate has to be written
	/// 
	/// Bind and Unbind handles checking when an entity is added or removed
	/// This is an immediate function call. If we need a delayed effect then
	/// I'll change to smth else.
	/// 
	/// calling system(dt) used for updating entities in the system
	/// 
	/// </summary>
	template<class SystemTag, class... Required>
	class BaseSystem : public IBaseSystem {

	public:
		void Bind(entt::registry& reg) override
		{
			mRegistry = &reg;
			(reg.on_construct<Required>().connect<&BaseSystem::SystemOnEnter>(*this), ...);
			(reg.on_destroy<Required>().connect<&BaseSystem::SystemOnExit>(*this), ...);
		}

		void Unbind() override
		{
			if (!mRegistry)
				return;

			(mRegistry->on_construct<Required>().disconnect<&BaseSystem::SystemOnEnter>(*this), ...);
			(mRegistry->on_destroy<Required>().disconnect<&BaseSystem::SystemOnExit>(*this), ...);
			mRegistry = nullptr;
		}

		void Update(float dt) override
		{
			auto& reg = *mRegistry;
			auto view = reg.view<SystemTag, Required...>();
			for (auto entity : view)
			{
				EntityOnUpdate(reg, entity, dt);
			}
		}

		~BaseSystem()
		{
			Unbind();
		}

		virtual void EntityOnEnter(entt::registry& reg, entt::entity entity) {}

		virtual void EntityOnExit(entt::registry& reg, entt::entity entity) {}

		virtual void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) {}

		entt::registry* mRegistry{};
	private:
		void SystemOnEnter(entt::registry& reg, entt::entity entity)
		{
			if (reg.all_of<Required...>(entity) && !reg.any_of<SystemTag>(entity))
			{
				reg.emplace<SystemTag>(entity);
				EntityOnEnter(reg, entity);
			}
		}

		void SystemOnExit(entt::registry& reg, entt::entity entity)
		{
			if (reg.any_of<SystemTag>(entity))
			{
				EntityOnExit(reg, entity);
				reg.remove<SystemTag>(entity);
			}
		}

	};

	/// <summary>
	/// Used for systems that don't require components or entities
	/// i.e ResourceManager? RenderManager?
	/// TODO: Check w chase on this
	/// </summary>
	class BaseEngineSystem : public IBaseSystem
	{
	public:
		void Bind(Registry& reg) override
		{
			mRegistry = &reg;
		}

		void Unbind() override
		{

		}

		void Update(float dt) override
		{

		}
	private:
		entt::registry* mRegistry{};
	};
}

#endif