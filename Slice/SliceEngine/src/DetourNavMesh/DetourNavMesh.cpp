#include <pch.h>
#include "DetourNavMesh.h"

namespace SliceEngine
{

    DetourNavMesh::~DetourNavMesh()
    {
        if (navQuery) dtFreeNavMeshQuery(navQuery);
        if (navMesh) dtFreeNavMesh(navMesh);
    }

    bool DetourNavMesh::LoadFromFile(const std::string &filePath)
    {
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open())
            return false;

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<unsigned char> buffer(size);
        if (!file.read(reinterpret_cast<char *>(buffer.data()), size))
            return false;

        navMesh = dtAllocNavMesh();
        if (dtStatusFailed(navMesh->init(buffer.data(), (int)size, DT_TILE_FREE_DATA)))
        {
            std::cerr << "Failed to init Detour navmesh from file!" << std::endl;
            return false;
        }

        navQuery = dtAllocNavMeshQuery();
        navQuery->init(navMesh, 2048);
        std::cout << "NavMesh loaded successfully!" << std::endl;

        return true;
    }

    bool DetourNavMesh::FindPath(const float *start, const float *end, std::vector<glm::vec3> &outPath)
    {
        if (!navQuery)
            return false;

        dtQueryFilter filter;
        filter.setIncludeFlags(0xFFFF);
        filter.setExcludeFlags(0);

        float polyPickExt[3] = { 2, 4, 2 };

        dtPolyRef startRef, endRef;
        float startPos[3], endPos[3];
        dtVcopy(startPos, start);
        dtVcopy(endPos, end);

        navQuery->findNearestPoly(startPos, polyPickExt, &filter, &startRef, nullptr);
        navQuery->findNearestPoly(endPos, polyPickExt, &filter, &endRef, nullptr);

        if (!startRef || !endRef)
            return false;

        dtPolyRef polys[256];
        int numPolys = 0;
        float straightPath[256 * 3];
        unsigned char straightPathFlags[256];
        dtPolyRef straightPathPolys[256];
        int straightPathCount = 0;

        if (dtStatusFailed(navQuery->findPath(startRef, endRef, startPos, endPos, &filter, polys, &numPolys, 256)))
            return false;

        if (numPolys)
        {
            navQuery->findStraightPath(
                startPos, endPos,
                polys, numPolys,
                straightPath, straightPathFlags, straightPathPolys,
                &straightPathCount, 256
            );

            outPath.clear();
            for (int i = 0; i < straightPathCount; ++i)
            {
                outPath.emplace_back(straightPath[i * 3 + 0],
                    straightPath[i * 3 + 1],
                    straightPath[i * 3 + 2]);
            }
            return true;
        }

        return false;
    }
}