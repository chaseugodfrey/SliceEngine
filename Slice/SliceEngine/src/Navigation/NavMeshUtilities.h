/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        DetourNavMesh.h

 author:	  Crystal Koh Qiao Wei

 email:       k.crystalqiaowei@digipen.edu

 brief:		 Header file for DetourNavMesh.cpp

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef DT_NAVMESH_H
#define DT_NAVMESH_H

#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>
#include <DetourCommon.h>

namespace SliceEngine
{
    namespace NavMeshUtilities
    {
        float distancePtLine2d(const float* pt, const float* p, const float* q);

        std::optional<NavMeshObj> LoadNavMesh(const std::string &filePath);
        NavMeshDebugObj CreateDebugMesh(NavMeshObj const& navMeshObj);

        bool FindPath(NavMeshObj& navMeshObj, const float *start, const float *end, std::vector<glm::vec3> &outPath);
    }
}
#endif