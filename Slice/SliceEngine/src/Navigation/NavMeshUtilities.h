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
#include <DetourCrowd.h>

namespace SliceEngine
{
    namespace NavMeshUtilities
    {
        float distancePtLine2d(const float* pt, const float* p, const float* q);
        dtCrowd *InitCrowd(dtNavMesh *navMesh);
        std::optional<NavMeshObj> LoadNavMesh(const std::string &filePath);
        std::optional<NavMeshDebugObj> CreateDebugMesh(NavMeshObj const& navMeshObj);

        NavMeshDebugObj CreateDebugPathMesh(const std::vector<glm::vec3> &pathPoints);

        bool FindPath(NavMeshObj& navMeshObj, const float *start, const float *end, std::vector<glm::vec3> &outPath);
        bool GetNavMeshHeightAtPos(NavMeshObj &navMeshObj, glm::vec3 pos, float &outHeight);
    }
}
#endif