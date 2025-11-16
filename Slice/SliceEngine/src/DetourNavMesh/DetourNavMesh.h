/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        DetourNavMesh.h

 author:	  Crystal Koh Qiao Wei

 email:       k.crystalqiaowei@digipen.edu

 brief:		 Header file for DetourNavMesh.cpp

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
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