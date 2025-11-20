#pragma once
#ifndef NAVIGATION_SYSTEM_H
#define NAVIGATION_SYSTEM_H

#include "ECS/BaseSystem.h"
#include "ECS/ECSTypes.h"
#include "NavMeshUtilities.h"

namespace SliceEngine
{

    struct NavAgentEntity {};

    class NavigationSystem : public BaseSystem<NavAgentEntity, NavAgent>
    {
        std::optional<NavMeshObj> navMeshInstance;
        std::optional<NavMeshDebugObj> navMeshDebugInfo;

    public:
        void Init();
        //void Update(float dt);
        void Unbind() override;

        void ClearNavMesh();
        void LoadNavMeshOnSceneLoad(OnSceneLoadedEvent& e);
        void LoadNavMeshFromBake(NavMeshObj obj);
        void LoadNavMeshFromFile();
        std::optional<NavMeshObj>& GetNavMeshObj();
        std::optional<NavMeshDebugObj>& GetNavMeshDebugData();

        void EntityOnEnter(entt::registry &reg, entt::entity entity) override;
        void EntityOnExit(entt::registry &reg, entt::entity entity) override;
        void EntityOnUpdate(entt::registry &reg, entt::entity entity, float dt) override;

    };
}


#endif