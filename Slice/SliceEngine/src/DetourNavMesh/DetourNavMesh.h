#ifndef DETOUR_NAVMESH_H
#define DETOUR_NAVMESH_H

#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>
#include <DetourCommon.h>
namespace SliceEngine
{
    class DetourNavMesh
    {
    public:
        DetourNavMesh() = default;
        ~DetourNavMesh();

        bool LoadFromFile(const std::string &filePath);
        bool FindPath(const float *start, const float *end, std::vector<glm::vec3> &outPath);

    private:
        dtNavMesh *navMesh = nullptr;
        dtNavMeshQuery *navQuery = nullptr;
    };
}
#endif