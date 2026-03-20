/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			BaseSystem.h
 author:		Gideon Francis
 email:			g.francis@digipen.edu
 brief:			Base Systems to be inherited by other systems

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
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
			(reg.on_construct<Required>().connect<&BaseSystem::OnRequiredAdded>(*this), ...);
			(reg.on_destroy<Required>().connect<&BaseSystem::OnRequiredRemoved>(*this), ...);

			reg.on_construct<SystemTag>().connect<&BaseSystem::OnTagAdded>(*this);
			reg.on_destroy<SystemTag>().connect<&BaseSystem::OnTagRemoved>(*this);
		}

		void Unbind() override
		{
			if (!mRegistry)
				return;

			(mRegistry->on_construct<Required>().disconnect<&BaseSystem::OnRequiredAdded>(*this), ...);
			(mRegistry->on_destroy<Required>().disconnect<&BaseSystem::OnRequiredRemoved>(*this), ...);

			mRegistry->on_construct<SystemTag>().disconnect<&BaseSystem::OnTagAdded>(*this);
			mRegistry->on_destroy<SystemTag>().disconnect<&BaseSystem::OnTagRemoved>(*this);			
			mRegistry = nullptr;
		}

		void Update(float dt) override
		{
			auto& reg = *mRegistry;
			auto view = reg.view<SystemTag, Required...>();
			for (auto entity : view)
			{
				if (reg.any_of<SliceEntity>(entity))
				{
					auto& sliceEntity = reg.get<SliceEntity>(entity);
					if (!sliceEntity.mActive)
						continue;
				}
				EntityOnUpdate(reg, entity, dt);
			}
		}

		~BaseSystem()
		{
			//Unbind();
		}

		virtual void EntityOnEnter(entt::registry& reg, entt::entity entity) = 0;

		virtual void EntityOnExit(entt::registry& reg, entt::entity entity) = 0;

		virtual void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) = 0;

		entt::registry* mRegistry{};
	private:

		void PrintComponents(entt::registry& reg, entt::entity entity)
		{
			//std::cout << "--- Components on Entity " << static_cast<uint32_t>(entity) << " ---" << std::endl;

			// Go through every registered component
			for (auto&& [type_id, storage] : reg.storage())
			{
				if (!storage.contains(entity))
				{
					continue; // entity does not have this component
				}

				//std::cout << " - " << storage.type().name() << std::endl;
			}
			//std::cout << "-------------------------------------------" << std::endl;
		}

		void OnRequiredAdded(entt::registry& reg, entt::entity entity)
		{
			if (reg.all_of<Required...>(entity) && !reg.any_of<SystemTag>(entity))
			{
				reg.emplace<SystemTag>(entity);
			}
		}

		void OnRequiredRemoved(entt::registry& reg, entt::entity entity)
		{
			if (reg.any_of<SystemTag>(entity))
			{
				reg.remove<SystemTag>(entity);
			}
		}

		void OnTagAdded(entt::registry& reg, entt::entity entity)
		{
			EntityOnEnter(reg, entity);
		}

		void OnTagRemoved(entt::registry& reg, entt::entity entity)
		{
			EntityOnExit(reg, entity);
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