#ifndef WORLD_SPACE_GRAPHICS_SYSTEM_H
#define WORLD_SPACE_GRAPHICS_SYSTEM_H

//#include "ResourceManager.h"
#include "Resource/ResourceManager.h"
#include "Resource/Resource.h"

#include "../ECS/BaseSystem.h"
#include "../ECS/ECSTypes.h"
#include "ECS/GameObject.h"

namespace SliceEngine
{
	struct renderEntity {};
	struct visibleEntity {};

	class ResourceManager;
	struct WorldSpaceGraphicsSystem : BaseSystem<renderEntity, Transform, Renderer>
	{
		void UseShader(ResourceManager* rcManager);
		void Update(float dt) override;
		void Render(ResourceManager* rcManager, Entity cam);

		void EntityOnEnter(entt::registry& reg, Entity entity) override;
		void EntityOnExit(entt::registry& reg, Entity entity) override;
		void EntityOnUpdate(entt::registry& reg, Entity entity, float dt) override;

		void EntityDraw(const Entity& entity);

		void FetchFrustrumCull(Entity camObj);
		void AddGridEntities(std::unordered_set<Entity>& in, const int& x, const int& z);

	private:
		Handle<SliceEngineTypes::Shader> mShader;
		Handle<SliceEngineTypes::Model> tempModel;
		static constexpr float gridSize = 10.f;
		static constexpr int gridNum = 11; // Has to be odd number lol (cuz account for 0, then +- halfGridNum)
		std::array<std::vector<Entity>, gridNum* gridNum> spatialData;
		std::vector<Entity> outerSpatial;

		void ResetVisibleEntities();
	};
}

#endif