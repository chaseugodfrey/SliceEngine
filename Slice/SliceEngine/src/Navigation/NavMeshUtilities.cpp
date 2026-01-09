/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        DetourNavMesh.cpp

 author:	  Crystal Koh Qiao Wei

 email:       k.crystalqiaowei@digipen.edu

 brief:		  Responsible for handling of the loading of and other functions of DetourNavMesh

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "NavMeshUtilities.h"
#include <filesystem>
namespace SliceEngine
{
	float NavMeshUtilities::distancePtLine2d(const float* pt, const float* p, const float* q)
	{
		float pqx = q[0] - p[0];
		float pqz = q[2] - p[2];
		float dx = pt[0] - p[0];
		float dz = pt[2] - p[2];
		float d = pqx * pqx + pqz * pqz;
		float t = pqx * dx + pqz * dz;
		if (d != 0) t /= d;
		dx = p[0] + t * pqx - pt[0];
		dz = p[2] + t * pqz - pt[2];
		return dx * dx + dz * dz;
	}

	dtCrowd *NavMeshUtilities::InitCrowd(dtNavMesh *navMesh)
	{
		dtCrowd *crowd = dtAllocCrowd();
		if (!crowd) return nullptr;

		// Max Agents: 50, Max Radius: 2.0f (may need to edit to cover agent size)
		if (!crowd->init(50, 2.0f, navMesh))
		{
			dtFreeCrowd(crowd);
			return nullptr;
		}

		// tune obstacle avoidance here if needed
		return crowd;
	}

	std::optional<NavMeshObj> NavMeshUtilities::LoadNavMesh(const std::string &filePath)
    {
        dtNavMesh *navMesh;
        dtNavMeshQuery *navQuery;

		std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open())
            return {};

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

		unsigned char* data = (unsigned char*)dtAlloc(static_cast<int>(size), DT_ALLOC_PERM);


		if (!file.read(reinterpret_cast<char*>(data), size))
		{
			dtFree(data);
			return {};
		}

        navMesh = dtAllocNavMesh();
        if (dtStatusFailed(navMesh->init(data, (int)size, DT_TILE_FREE_DATA)))
        {
            std::cerr << "Failed to init Detour navmesh from file!" << std::endl;
			dtFree(data);
            dtFreeNavMesh(navMesh);
            return {};
        }

        navQuery = dtAllocNavMeshQuery();
        navQuery->init(navMesh, 2048); 
		//dtCrowd *crowd = InitCrowd(navMesh);

		std::cout << "NavMesh loaded successfully!" << std::endl;

        return { NavMeshObj{navMesh, navQuery/*, crowd*/}};
    }

    bool NavMeshUtilities::FindPath(NavMeshObj& navMeshObj, const float *start, const float *end, std::vector<glm::vec3> &outPath)
    {

		if (!navMeshObj.navMesh)
		{
			std::cerr << "ERROR: navMesh is null!" << std::endl;
			return false;
		}
		if (!navMeshObj.navMeshQuery)
		{
			std::cerr << "ERROR: navMeshQuery is null!" << std::endl;
			return false;
		}

		// Check if navmesh has any tiles using public API
		int tileCount = navMeshObj.navMesh->getMaxTiles();
		std::cout << "NavMesh has " << tileCount << " max tiles" << std::endl;

		// Check tile count a different way
		const dtNavMeshParams *params = navMeshObj.navMesh->getParams();
		if (params)
		{
			std::cout << "NavMesh params - maxTiles: " << params->maxTiles
				<< " maxPolys: " << params->maxPolys << std::endl;
		}

        dtQueryFilter filter;
        filter.setIncludeFlags(0xFFFF);
        filter.setExcludeFlags(0);
        //filter.setAreaCost(SAMPLE_POLYAREA_GROUND, 1.0f);

        float polyPickExt[3] = { 10, 10, 10 };
        dtPolyRef startRef, endRef;

        //float startPos[3], endPos[3];
        //dtVcopy(startPos, start);
        //dtVcopy(endPos, end);

        //navMeshObj.navMeshQuery->findNearestPoly(startPos, polyPickExt, &filter, &startRef, nullptr);
        //navMeshObj.navMeshQuery->findNearestPoly(endPos, polyPickExt, &filter, &endRef, nullptr);

		// In your FindPath function, add debug logging:
		float startPos[3], endPos[3];
		dtVcopy(startPos, start);
		dtVcopy(endPos, end);

		std::cout << "Searching for start: (" << startPos[0] << ", " << startPos[1] << ", " << startPos[2] << ")" << std::endl;
		std::cout << "Searching for end: (" << endPos[0] << ", " << endPos[1] << ", " << endPos[2] << ")" << std::endl;

		float nearestStart[3], nearestEnd[3];
		navMeshObj.navMeshQuery->findNearestPoly(startPos, polyPickExt, &filter, &startRef, nearestStart);
		navMeshObj.navMeshQuery->findNearestPoly(endPos, polyPickExt, &filter, &endRef, nearestEnd);

		if (!startRef)
		{
			std::cerr << "Start not found. Nearest would be: ("
				<< nearestStart[0] << ", " << nearestStart[1] << ", " << nearestStart[2] << ")" << std::endl;
		}
		if (!endRef)
		{
			std::cerr << "End not found. Nearest would be: ("
				<< nearestEnd[0] << ", " << nearestEnd[1] << ", " << nearestEnd[2] << ")" << std::endl;
		}


        if (!startRef || !endRef)
        {
            std::cerr << "Failed to find nearest polygons for start or end." << std::endl;
            return false;
        }

        dtPolyRef polys[256];
        int numPolys = 0;
        if (dtStatusFailed(navMeshObj.navMeshQuery->findPath(startRef, endRef, startPos, endPos, &filter, polys, &numPolys, 256)))
            return false;

        if (!numPolys)
            return false;

        float straightPath[256 * 3];
        unsigned char straightPathFlags[256];
        dtPolyRef straightPathPolys[256];
        int straightPathCount = 0;

        navMeshObj.navMeshQuery->findStraightPath(
            startPos, endPos,
            polys, numPolys,
            straightPath, straightPathFlags, straightPathPolys,
            &straightPathCount, 256,
            DT_STRAIGHTPATH_AREA_CROSSINGS
        );

        outPath.clear();
        for (int i = 0; i < straightPathCount; ++i)
        {
            outPath.emplace_back(
                straightPath[i * 3 + 0],
                straightPath[i * 3 + 1],
                straightPath[i * 3 + 2]
            );
        }

        return !outPath.empty();	
    }

	bool NavMeshUtilities::GetNavMeshHeightAtPos(NavMeshObj &navMeshObj, glm::vec3 pos, float &outHeight)
	{
		if (!navMeshObj.navMeshQuery) return false;

		float position[3] = { pos.x, pos.y, pos.z };
		float extents[3] = { 2.0f, 10.0f, 2.0f }; // Look 10 units up/down for the mesh

		dtQueryFilter filter; // Default filter
		dtPolyRef nearestPoly;
		float nearestPt[3];

		// 1. Find the polygon closest to our (x, z) position
		navMeshObj.navMeshQuery->findNearestPoly(position, extents, &filter, &nearestPoly, nearestPt);

		if (!nearestPoly) return false;

		// 2. Get the exact height of that polygon at our x, z coordinates
		if (dtStatusSucceed(navMeshObj.navMeshQuery->getPolyHeight(nearestPoly, position, &outHeight)))
		{
			return true;
		}

		// Fallback: if getPolyHeight fails (rare), use the nearest point's Y
		outHeight = nearestPt[1];
		return true;
	}

	NavMeshDebugObj NavMeshUtilities::CreateDebugMesh(NavMeshObj const& navMeshObj)
	{
		auto tNavMesh = const_cast<const dtNavMesh*>(navMeshObj.navMesh);

		if (tNavMesh)
		{
			std::vector<float> vertices;
			std::vector<float> verticesBoundaries;
			//std::vector<unsigned short> indices;
			for (int t{}; t < tNavMesh->getMaxTiles(); ++t)
			{
				const dtMeshTile* tile = tNavMesh->getTile(t);
				if (!tile->header) continue;

				//dtPolyRef  base = tNavMesh->getPolyRefBase(tile);
				for (int i{}; i < tile->header->polyCount; ++i)
				{
					const dtPoly* p = &tile->polys[i];
					if (p->getType() == DT_POLYTYPE_OFFMESH_CONNECTION)
						continue;
					const dtPolyDetail* pd = &tile->detailMeshes[i];

					// The Blue Floor
					for (int j{}; j < pd->triCount; ++j)
					{
						const unsigned char* z = &tile->detailTris[(pd->triBase + j) * 4];
						for (int k{}; k < 3; ++k)
						{
							if (z[k] < p->vertCount)
							{
								vertices.push_back(tile->verts[p->verts[z[k]] * 3]);
								vertices.push_back(tile->verts[p->verts[z[k]] * 3 + 1]);
								vertices.push_back(tile->verts[p->verts[z[k]] * 3 + 2]);
							}
							else
							{
								vertices.push_back(tile->detailVerts[(pd->vertBase + z[k] - p->vertCount) * 3]);
								vertices.push_back(tile->detailVerts[(pd->vertBase + z[k] - p->vertCount) * 3 + 1]);
								vertices.push_back(tile->detailVerts[(pd->vertBase + z[k] - p->vertCount) * 3 + 2]);
							}
						}
					}
					// The Dark Blue Boundaries
					for (int j{}, nj{ static_cast<int>(p->vertCount) }; j < nj; ++j)
					{
						// if Inner
						// else
						if (p->neis[j] != 0)
							continue;

						const float* v0 = &tile->verts[p->verts[j] * 3];
						const float* v1 = &tile->verts[p->verts[(j + 1) % nj] * 3];

						for (int k{}; k < pd->triCount; ++k)
						{
							const unsigned char* d = &tile->detailTris[(pd->triBase + k) * 4];
							const float* tv[3];
							for (int m{}; m < 3; ++m)
							{
								if (d[m] < p->vertCount)
									tv[m] = &tile->verts[p->verts[d[m]] * 3];
								else
									tv[m] = &tile->detailVerts[(pd->vertBase + (d[m] - p->vertCount)) * 3];
							}
							for (int m{}, n{ 2 }; m < 3; n = m++)
							{
								if ((dtGetDetailTriEdgeFlags(d[3], n) & DT_DETAIL_EDGE_BOUNDARY) == 0)
									continue;
								static const float thr = 0.01f * 0.01f;
								if (distancePtLine2d(tv[n], v0, v1) < thr &&
									distancePtLine2d(tv[m], v0, v1) < thr)
								{
									glm::vec2 dir(tv[m][0] - tv[n][0], tv[m][2] - tv[n][2]);
									dir = glm::normalize(dir) * 0.03f;
									std::swap(dir.x, dir.y);

									verticesBoundaries.push_back(tv[n][0] - dir.x);
									verticesBoundaries.push_back(tv[n][1] + 0.005f);
									verticesBoundaries.push_back(tv[n][2] - dir.y);

									verticesBoundaries.push_back(tv[n][0] + dir.x);
									verticesBoundaries.push_back(tv[n][1] + 0.005f);
									verticesBoundaries.push_back(tv[n][2] + dir.y);

									verticesBoundaries.push_back(tv[m][0] + dir.x);
									verticesBoundaries.push_back(tv[m][1] + 0.005f);
									verticesBoundaries.push_back(tv[m][2] + dir.y);

									verticesBoundaries.push_back(tv[m][0] + dir.x);
									verticesBoundaries.push_back(tv[m][1] + 0.01f);
									verticesBoundaries.push_back(tv[m][2] + dir.y);

									verticesBoundaries.push_back(tv[m][0] - dir.x);
									verticesBoundaries.push_back(tv[m][1] + 0.01f);
									verticesBoundaries.push_back(tv[m][2] - dir.y);

									verticesBoundaries.push_back(tv[n][0] + dir.x);
									verticesBoundaries.push_back(tv[n][1] + 0.01f);
									verticesBoundaries.push_back(tv[n][2] + dir.y);
								}
							}
						}
					}
				}
			}
			// ********************************************* Debug mesh *********************************************
			//auto& dNavMesh = SliceEngine::Core::GetInstance()->debugNavMesh;
			NavMeshDebugObj dataObjArr{};
			//vbo
			glCreateBuffers(1, &dataObjArr.data[0].vbo);
			glNamedBufferStorage(dataObjArr.data[0].vbo, vertices.size() * sizeof(float), vertices.data(), 0);

			//ebo
			//glCreateBuffers(1, &dNavMesh[0].ebo);
			//glNamedBufferStorage(dNavMesh[0].ebo, indices.size() * sizeof(unsigned short), indices.data(), 0);

			//vao
			glCreateVertexArrays(1, &dataObjArr.data[0].vao);
			glEnableVertexArrayAttrib(dataObjArr.data[0].vao, 0);
			glVertexArrayAttribFormat(dataObjArr.data[0].vao, 0, 3, GL_FLOAT, false, 0);
			//glVertexArrayElementBuffer(dNavMesh[0].vao, dNavMesh[0].ebo);

			glVertexArrayVertexBuffer(dataObjArr.data[0].vao, 0, dataObjArr.data[0].vbo, 0, sizeof(float) * 3);
			glVertexArrayAttribBinding(dataObjArr.data[0].vao, 0, 0);

			dataObjArr.data[0].drawCnt = static_cast<uint32_t>(vertices.size() / 3);
			// ********************************************* Boundaries *********************************************
			glCreateBuffers(1, &dataObjArr.data[1].vbo);
			glNamedBufferStorage(dataObjArr.data[1].vbo, verticesBoundaries.size() * sizeof(float), verticesBoundaries.data(), 0);

			//vao
			glCreateVertexArrays(1, &dataObjArr.data[1].vao);
			glEnableVertexArrayAttrib(dataObjArr.data[1].vao, 0);
			glVertexArrayAttribFormat(dataObjArr.data[1].vao, 0, 3, GL_FLOAT, false, 0);
			//glVertexArrayElementBuffer(dNavMesh[0].vao, dNavMesh[0].ebo);

			glVertexArrayVertexBuffer(dataObjArr.data[1].vao, 0, dataObjArr.data[1].vbo, 0, sizeof(float) * 3);
			glVertexArrayAttribBinding(dataObjArr.data[1].vao, 0, 0);

			dataObjArr.data[1].drawCnt = static_cast<uint32_t>(verticesBoundaries.size() / 3);

			return dataObjArr;
		}
	}

	NavMeshDebugObj NavMeshUtilities::CreateDebugPathMesh(const std::vector<glm::vec3> &pathPoints)
	{
		NavMeshDebugObj dataObjArr{};
		std::vector<float> vertices;

		// Convert glm::vec3 path to float array for rendering
		// Raise the line slightly (Y+0.1) so it draws on top of the NavMesh
		for (const auto &point : pathPoints)
		{
			vertices.push_back(point.x);
			vertices.push_back(point.y + 0.2f);
			vertices.push_back(point.z);
		}

		// Use the first slot (data[0]) for the path line
		glCreateBuffers(1, &dataObjArr.data[0].vbo);
		glNamedBufferStorage(dataObjArr.data[0].vbo, vertices.size() * sizeof(float), vertices.data(), 0);

		glCreateVertexArrays(1, &dataObjArr.data[0].vao);
		glEnableVertexArrayAttrib(dataObjArr.data[0].vao, 0);
		glVertexArrayAttribFormat(dataObjArr.data[0].vao, 0, 3, GL_FLOAT, false, 0);

		glVertexArrayVertexBuffer(dataObjArr.data[0].vao, 0, dataObjArr.data[0].vbo, 0, sizeof(float) * 3);
		glVertexArrayAttribBinding(dataObjArr.data[0].vao, 0, 0);

		dataObjArr.data[0].drawCnt = static_cast<uint32_t>(vertices.size() / 3);

		return dataObjArr;
	}
}