/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        RecastNavmesh.h

 author:      Crystal Koh Qiao Wei

 email:       k.crystalqiaowei@digipen.edu

 brief:       Defines the RecastNavMesh class, responsible for extracting level geometry
              and baking it into a Navigation Mesh using the Recast library. It also
              initializes the Detour navigation data and query objects required for
              runtime pathfinding within the editor and engine.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef RECAST_NAVMESH_H
#define RECAST_NAVMESH_H

#include <string>
#include <vector>
#include <Recast.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>
#include "Resource/Model.h"
#include <DetourNavMeshQuery.h>
// need detour libs so i can send the data into detour, which can be used in engine side
namespace SliceEditor
{

	class RecastNavMesh
	{
	public:
		RecastNavMesh();
		~RecastNavMesh();
        /**
         * @brief Extracts geometry from a single entity and bakes a Navigation Mesh.
         * * @param entity Pointer to the entity containing the model geometry to build from.
         * @return True if the NavMesh was successfully built, false otherwise.
         */
        bool BuildFromModel(Entity* entity);

        /**
         * @brief Extracts and combines geometry from multiple entities to bake a unified Navigation Mesh.
         * * @param entities A vector of entity pointers containing the models to build from.
         * @return True if the NavMesh was successfully built, false otherwise.
         */
        bool BuildFromModel(
            const std::vector<Entity *> entities
        );
        
        /**
         * @brief Recursively traverses a model's node hierarchy to extract vertices and indices.
         * Transforms vertices into world space and flattens them into single buffers for Recast.
         * * @param model           The model containing the geometry.
         * @param node            The current node being processed.
         * @param parentTransform The accumulated local-to-world transformation matrix.
         * @param outVertices     Buffer storing the flattened world-space vertices.
         * @param outIndices      Buffer storing the flattened indices.
         * @param vertexOffset    The current index offset to ensure global indices are correct across multiple meshes.
         */
        void CollectMeshDataFromNode(
            const SliceEngine::SliceEngineTypes::Model &model,
            const SliceEngine::SliceEngineTypes::ModelNode &node,
            const glm::mat4 &parentTransform,
            std::vector<SliceEngine::SliceEngineTypes::Vertex> &outVertices,
            std::vector<unsigned int> &outIndices,
            size_t &vertexOffset);

        dtNavMeshQuery *GetNavMeshQuery() { return navQuery; }
        dtNavMesh *GetNavMesh() { return navMesh; }
        void Init();
        void Clear();

        rcConfig& GetConfig();
        float *GetAgentHeight();
        float *GetAgentRadius();
        float *GetMaxClimb();

    private:
        rcConfig config{};
        rcPolyMeshDetail *detailMesh = nullptr;

        rcHeightfield *heightfield = nullptr;
        rcCompactHeightfield *compactHeightfield = nullptr;
        rcContourSet *contourSet = nullptr;
        rcPolyMesh *polyMesh = nullptr;

        dtNavMesh *navMesh = nullptr;      
        dtNavMeshQuery *navQuery = nullptr;

        float m_agentHeight{};
        float m_agentRadius{};
        float m_agentMaxClimb{};
	};
}
#endif