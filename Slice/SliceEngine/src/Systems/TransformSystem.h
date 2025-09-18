#ifndef TRANSFORM_MANAGER_H
#define TRANSFORM_MANAGER_H

#include "ECS/BaseSystem.h"
#include "ECS/ECSTypes.h"
#include "ECS/GameObject.h"

namespace SliceEngine
{
	struct transformEntity {};

	struct TransformSystem : BaseSystem<transformEntity, Transform>
	{
		void EntityOnEnter(entt::registry& reg, entt::entity entity) override;
		void EntityOnExit(entt::registry& reg, entt::entity entity) override;
		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override;

		void Update(float dt) override;

		std::unordered_set<Entity> FetchFrustrumCull(GameObject& camObj);

		void AddGridEntities(std::unordered_set<Entity>& in, const int& x, const int& z);

	private:
		static constexpr float gridSize = 10.f;
		static constexpr int gridNum = 11; // Has to be odd number lol (cuz account for 0, then +- halfGridNum)
		std::array<std::vector<Entity>, gridNum * gridNum> spatialData;
		std::vector<Entity> outerSpatial;
	};
}


#endif