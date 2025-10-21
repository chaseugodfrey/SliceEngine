#include <pch.h>
#include "RecastNavMesh.h"
#include <cstring>
#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>
#include <DetourNavMeshQuery.h>
#include <DetourCommon.h>

namespace SliceEditor
{
	RecastNavMesh::RecastNavMesh() {}

	RecastNavMesh::~RecastNavMesh()
	{
		Clear();
	}

	void RecastNavMesh::Clear()
	{
		if (navQuery) dtFreeNavMeshQuery(navQuery);
		if (navMesh) dtFreeNavMesh(navMesh);

		if (polyMesh) rcFreePolyMesh(polyMesh);
		if (contourSet) rcFreeContourSet(contourSet);
		if (compactHeightfield) rcFreeCompactHeightfield(compactHeightfield);
		if (heightfield) rcFreeHeightField(heightfield);

		navQuery = nullptr;
		navMesh = nullptr;
		polyMesh = nullptr;
		contourSet = nullptr;
		compactHeightfield = nullptr;
		heightfield = nullptr;
	}

	bool RecastNavMesh::BuildFromModel(const SliceEngine::SliceEngineTypes::Model &model, const glm::mat4 &transform)
	{
		Clear(); // start fresh

		const auto &vertices = model.meshes[0].vertices;
		const auto &indices = model.meshes[0].indices;

		//std::cout << "Input indices count: " << vertices.size() << std::endl;
		//for (auto &v : indices)
		//	std::cout << "(" << v << ")\n";
		if (vertices.empty() || indices.empty())
			return false;

		// Convert vertices to Recast format
		std::vector<float> verts;
		verts.reserve(vertices.size() * 3);

		for (size_t i = 0; i < vertices.size(); ++i)
		{
			const auto &v = vertices[i].position;

			// Apply transformation
			glm::vec4 worldPos = transform * glm::vec4(v.x, v.y, v.z, 1.0f);

			// Convert to Recast coordinate system (swap Y and Z)
			verts.push_back(worldPos.x);
			verts.push_back(worldPos.y);  // Z becomes Y (height)
			verts.push_back(worldPos.z);  // Y becomes Z (depth)
		}

		std::cout << "Input vertex count: " << vertices.size() << std::endl;
		for (auto &v : verts)
			std::cout << "(" << v << ")\n";


		std::cout << "indices.size(): " << indices.size() << std::endl;
		std::cout << "verts.size(): " << verts.size() << std::endl;
		// Create a new indices array for Recast (0..n-1)
		std::vector<int> recastIndices(indices.begin(), indices.end());

		memset(&config, 0, sizeof(config));
		config.cs = 0.2f;  // 20cm cells - reasonable for navigation
		config.ch = 0.2f;  // 20cm height
		config.walkableHeight = (int)ceilf(2.0f / config.ch);   // 10 cells = 2 units height
		config.walkableClimb = (int)floorf(0.5f / config.ch);   // 2-3 cells = 0.5 units climb
		config.walkableRadius = (int)ceilf(0.4f / config.cs);   // 2 cells = 0.4 units radius
		config.maxEdgeLen = (int)(12.0f / config.cs);            // 60 cells
		config.maxSimplificationError = 1.3f;
		config.minRegionArea = (int)rcSqr(8);   // Back to reasonable values
		config.mergeRegionArea = (int)rcSqr(20);
		config.maxVertsPerPoly = 6;
		config.detailSampleDist = config.cs * 6.0f;
		config.detailSampleMaxError = config.ch * 1.0f;

		// Bounding box
		float bmin[3], bmax[3];
		rcCalcBounds(verts.data(), (int)(verts.size() / 3), bmin, bmax);

		// ADD THIS DEBUG
		std::cout << "Bounds: (" << bmin[0] << "," << bmin[1] << "," << bmin[2] << ") to ("
			<< bmax[0] << "," << bmax[1] << "," << bmax[2] << ")" << std::endl;

		// FIX: Ensure minimum bounds in Y (height) direction
		if (bmax[1] - bmin[1] < 0.01f)
		{
			bmax[1] = bmin[1] + 0.01f;  // Add at least 1cm of height
		}

		rcCalcGridSize(bmin, bmax, config.cs, &config.width, &config.height);

		rcContext ctx;
		heightfield = rcAllocHeightfield();
		if (!heightfield) return false;

		if (!rcCreateHeightfield(&ctx, *heightfield, config.width, config.height, bmin, bmax, config.cs, config.ch))
			return false;

		std::vector<unsigned char> areas(recastIndices.size() / 3, RC_WALKABLE_AREA);

		// Debug: Check triangle winding
		for (int t = 0; t < (int)(recastIndices.size() / 3); ++t)
		{
			int i0 = recastIndices[t * 3 + 0] * 3;
			int i1 = recastIndices[t * 3 + 1] * 3;
			int i2 = recastIndices[t * 3 + 2] * 3;

			float v0[3] = { verts[i0], verts[i0 + 1], verts[i0 + 2] };
			float v1[3] = { verts[i1], verts[i1 + 1], verts[i1 + 2] };
			float v2[3] = { verts[i2], verts[i2 + 1], verts[i2 + 2] };

			// Calculate normal
			float e0[3] = { v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2] };
			float e1[3] = { v2[0] - v0[0], v2[1] - v0[1], v2[2] - v0[2] };
			float normal[3] = {
				e0[1] * e1[2] - e0[2] * e1[1],
				e0[2] * e1[0] - e0[0] * e1[2],
				e0[0] * e1[1] - e0[1] * e1[0]
			};

			std::cout << "Triangle " << t << " normal: (" << normal[0] << ", " << normal[1] << ", " << normal[2] << ")" << std::endl;
		}

		rcRasterizeTriangles(&ctx, verts.data(), (int)verts.size() / 3,
			recastIndices.data(), areas.data(), (int)(recastIndices.size() / 3),
			*heightfield, config.walkableClimb);

		std::cout << "Heightfield width: " << heightfield->width
			<< " height: " << heightfield->height << std::endl;
		int spanCount = 0;
		for (int i = 0; i < heightfield->width * heightfield->height; ++i)
		{
			for (rcSpan *s = heightfield->spans[i]; s; s = s->next)
			{
				spanCount++;
			}
		}
		std::cout << "Heightfield span count: " << spanCount << std::endl;

		//rcFilterLowHangingWalkableObstacles(&ctx, config.walkableClimb, *heightfield);
		//rcFilterLedgeSpans(&ctx, config.walkableHeight, config.walkableClimb, *heightfield);
		//rcFilterWalkableLowHeightSpans(&ctx, config.walkableHeight, *heightfield);

		compactHeightfield = rcAllocCompactHeightfield();
		if (!compactHeightfield) return false;
		if (!rcBuildCompactHeightfield(&ctx, config.walkableHeight, config.walkableClimb, *heightfield, *compactHeightfield))
			return false;
		
		std::cout << "Compact heightfield span count: " << compactHeightfield->spanCount << std::endl;
		std::cout << "Compact heightfield max region count: " << compactHeightfield->maxRegions << std::endl;
		if (!rcBuildDistanceField(&ctx, *compactHeightfield))
			return false;

		if (!rcBuildRegions(&ctx, *compactHeightfield, 0, config.minRegionArea, config.mergeRegionArea))
			return false;

		std::cout << "Max region: " << compactHeightfield->maxRegions << std::endl;

		// Now check if we actually have regions
		if (compactHeightfield->maxRegions == 0)
		{
			std::cout << "ERROR: No regions were created!" << std::endl;
			return false;
		}


		//rcErodeWalkableArea(&ctx, config.walkableRadius, *compactHeightfield);

		contourSet = rcAllocContourSet();
		if (!contourSet) return false;
		if (!rcBuildContours(&ctx, *compactHeightfield, config.maxSimplificationError, config.maxEdgeLen, *contourSet))
			return false;
		std::cout << "Contour count: " << contourSet->nconts << std::endl;


		polyMesh = rcAllocPolyMesh();
		if (!polyMesh) return false;
		if (!rcBuildPolyMesh(&ctx, *contourSet, config.maxVertsPerPoly, *polyMesh))
			return false;
		std::cout << "PolyMesh nverts: " << polyMesh->nverts << " npolys: " << polyMesh->npolys << std::endl;
		rcPolyMeshDetail *detailMesh = nullptr;
		detailMesh = rcAllocPolyMeshDetail();
		rcBuildPolyMeshDetail(&ctx, *polyMesh, *compactHeightfield, config.detailSampleDist, config.detailSampleMaxError, *detailMesh);

		// Fill dtNavMeshCreateParams
		dtNavMeshCreateParams params{};
		memset(&params, 0, sizeof(params));
		params.verts = polyMesh->verts; 
		params.vertCount = polyMesh->nverts;
		params.polys = polyMesh->polys;
		params.polyAreas = polyMesh->areas;
		params.polyFlags = polyMesh->flags;
		params.polyCount = polyMesh->npolys;
		params.nvp = polyMesh->nvp;
		params.detailMeshes = detailMesh->meshes; 
		params.detailVerts = detailMesh->verts;
		params.detailVertsCount = detailMesh->nverts;
		params.detailTris = detailMesh->tris;
		params.detailTriCount = detailMesh->ntris;
		rcVcopy(params.bmin, polyMesh->bmin);
		rcVcopy(params.bmax, polyMesh->bmax);

		params.cs = config.cs;
		params.ch = config.ch;
		params.buildBvTree = true;

		unsigned char *navData = nullptr;
		int navDataSize = 0;
		std::cout << "verts: " << params.vertCount
			<< " polys: " << params.polyCount
			<< " nvp: " << params.nvp
			<< " detailVerts: " << params.detailVertsCount
			<< " detailTris: " << params.detailTriCount
			<< std::endl;
		if (!dtCreateNavMeshData(&params, &navData, &navDataSize)) return false; // returning false here

		navMesh = dtAllocNavMesh();
		if (dtStatusFailed(navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA)))
		{
			dtFree(navData);
			return false;
		}

		navQuery = dtAllocNavMeshQuery();
		navQuery->init(navMesh, 2048);

		return true;
	}
}
