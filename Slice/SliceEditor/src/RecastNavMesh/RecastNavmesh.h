#ifndef MESH_H
#define MESH_H

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

        bool BuildFromModel(const SliceEngine::SliceEngineTypes::Model &model);
        dtNavMeshQuery *GetNavMeshQuery() { return navQuery; }
        dtNavMesh *GetNavMesh() { return navMesh; }
        void Clear();

    private:
        rcHeightfield *heightfield = nullptr;
        rcCompactHeightfield *compactHeightfield = nullptr;
        rcContourSet *contourSet = nullptr;
        rcPolyMesh *polyMesh = nullptr;
        rcConfig config{};

        dtNavMesh *navMesh = nullptr;      
        dtNavMeshQuery *navQuery = nullptr;
	};
}
#endif