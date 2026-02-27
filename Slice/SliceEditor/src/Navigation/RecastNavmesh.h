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

        bool BuildFromModel(Entity* entity);

        //bool BuildFromModel(
        //    const std::vector<Entity *> entities,
        //    const std::vector<SliceEngine::NavMeshLink> &links 
        //);

        bool BuildFromModel(
            const std::vector<Entity *> entities
        );
        // helper to look for child nodes
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