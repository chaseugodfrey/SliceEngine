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

        bool BuildFromModel(const SliceEngine::SliceEngineTypes::Model &model, const glm::mat4 &transform);
        bool BuildFromModel(const std::vector<SliceEngine::SliceEngineTypes::Model> &model, const std::vector<glm::mat4> &transform);
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
        void Clear();

    private:
        rcPolyMeshDetail *detailMesh = nullptr;

        rcHeightfield *heightfield = nullptr;
        rcCompactHeightfield *compactHeightfield = nullptr;
        rcContourSet *contourSet = nullptr;
        rcPolyMesh *polyMesh = nullptr;
        rcConfig config{};

        dtNavMesh *navMesh = nullptr;      
        dtNavMeshQuery *navQuery = nullptr;

        void ReleaseDebugMesh();
        void LoadDebugMesh();
	};
}
#endif