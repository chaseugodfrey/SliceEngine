/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        NavMeshUtilities.h

 author:      Crystal Koh Qiao Wei

 email:       k.crystalqiaowei@digipen.edu

brief:        Provides a collection of standalone utility functions for interacting with
              Detour Navigation Meshes. This includes functions for file I/O, generating
              renderable debug geometry for meshes and paths, calculating point-to-point
              paths, querying surface heights, and initializing crowd managers.

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
        /**
         * @brief Calculates the shortest distance from a point to a 2D line segment (typically on the XZ plane).
         * * @param pt The point to calculate the distance from.
         * @param p  The starting point of the line segment.
         * @param q  The ending point of the line segment.
         * @return The shortest distance from 'pt' to the segment 'pq'.
         */
        float distancePtLine2d(const float* pt, const float* p, const float* q);

        /**
         * @brief Initializes a Detour crowd manager for a given Navigation Mesh.
         * Useful for handling local steering and dynamic avoidance for multiple agents.
         * * @param navMesh Pointer to the initialized Detour NavMesh.
         * @return A pointer to the newly created dtCrowd instance.
         */
        dtCrowd *InitCrowd(dtNavMesh *navMesh);

        /**
         * @brief Deserializes and loads a previously baked Detour NavMesh from a binary file.
         * * @param filePath The file path to the saved NavMesh data.
         * @return An optional containing the loaded NavMeshObj, or std::nullopt if loading fails.
         */
        std::optional<NavMeshObj> LoadNavMesh(const std::string &filePath);

        /**
         * @brief Extracts renderable geometry (vertices and indices) from a Detour NavMesh.
         *
         * * @param navMeshObj The navigation mesh object to extract data from.
         * @return An optional containing the debug geometry ready for rendering.
         */
        std::optional<NavMeshDebugObj> CreateDebugMesh(NavMeshObj const& navMeshObj);

        /**
         * @brief Queries the NavMesh to find a path between a starting position and a destination.
         * * @param navMeshObj The navigation mesh object containing the query instance.
         * @param start      A float array (x, y, z) representing the starting world position.
         * @param end        A float array (x, y, z) representing the target world position.
         * @param outPath    A vector populated with the resulting world-space waypoints if successful.
         * @return True if a valid path was found, false otherwise.
         */
        bool FindPath(NavMeshObj& navMeshObj, const float *start, const float *end, std::vector<glm::vec3> &outPath);

        /**
         * @brief Queries the NavMesh polygon at a given position to find its exact Y-coordinate.
         * Highly useful for clamping or grounding characters directly to the surface of the NavMesh.
         * * @param navMeshObj The navigation mesh object to query.
         * @param pos        The world-space position (x, y, z) to check. The y-value is typically used as a search baseline.
         * @param outHeight  Reference to a float that will be populated with the resolved Y-height.
         * @return True if a polygon was found at the position and the height was successfully retrieved.
         */
        bool GetNavMeshHeightAtPos(NavMeshObj &navMeshObj, glm::vec3 pos, float &outHeight);
    }
}
#endif