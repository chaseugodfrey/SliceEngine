/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			WorldSpaceGraphicsSystem.h
 author:		Won Yu Xuan Rainne
 email:			won.m@digipen.edu
 brief:			Handles draw calls, and maybe spatial partioning of all renderEntities

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef WORLD_SPACE_GRAPHICS_SYSTEM_H
#define WORLD_SPACE_GRAPHICS_SYSTEM_H

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
		void Update(float dt) override;
		void Render(GLuint mShader, bool withTex);

		void EntityOnEnter(entt::registry& reg, Entity entity) override;
		void EntityOnExit(entt::registry& reg, Entity entity) override;
		void EntityOnUpdate(entt::registry& reg, Entity entity, float dt) override;

		void EntityDraw(const Entity& entity);

		void FetchFrustrumCull(Entity camObj);
		void AddGridEntities(std::unordered_set<Entity>& in, const int& x, const int& z);

	private:
		GLuint mShader;
		bool mHasRenderTexture;

		static constexpr float gridSize = 10.f;
		static constexpr int gridNum = 11; // Has to be odd number lol (cuz account for 0, then +- halfGridNum)
		std::array<std::vector<Entity>, gridNum* gridNum> spatialData;
		std::vector<Entity> outerSpatial;

		void ResetVisibleEntities();
	};
}

#endif