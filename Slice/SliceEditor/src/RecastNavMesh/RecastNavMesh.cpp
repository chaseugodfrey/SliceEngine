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

	bool RecastNavMesh::BuildFromModel(const SliceEngine::SliceEngineTypes::Model &model)
	{
		Clear(); // start fresh

		const auto &vertices = model.GetFinalVert();
		const auto &indices = model.GetFinalInd();

		if (vertices.empty() || indices.empty())
			return false;

		// Convert vertices to Recast format
		std::vector<float> verts;
		verts.reserve(vertices.size() * 3);
		for (const auto &v : vertices)
		{
			verts.push_back(v.position.x);
			verts.push_back(v.position.y);
			verts.push_back(v.position.z);
		}

		memset(&config, 0, sizeof(config));
		config.cs = 0.3f;
		config.ch = 0.2f;
		config.walkableSlopeAngle = 45.0f;
		config.walkableHeight = (int)ceilf(2.0f / config.ch);
		config.walkableClimb = (int)floorf(0.9f / config.ch);
		config.walkableRadius = (int)ceilf(0.6f / config.cs);
		config.maxEdgeLen = (int)(12.0f / config.cs);
		config.maxSimplificationError = 1.3f;
		config.minRegionArea = (int)rcSqr(8);
		config.mergeRegionArea = (int)rcSqr(20);
		config.maxVertsPerPoly = 6;
		config.detailSampleDist = config.cs * 6.0f;
		config.detailSampleMaxError = 1.0f;

		// Bounding box
		float bmin[3], bmax[3];
		rcCalcBounds(verts.data(), (int)vertices.size(), bmin, bmax);
		rcCalcGridSize(bmin, bmax, config.cs, &config.width, &config.height);

		rcContext ctx;
		heightfield = rcAllocHeightfield();
		if (!heightfield) return false;

		if (!rcCreateHeightfield(&ctx, *heightfield, config.width, config.height, bmin, bmax, config.cs, config.ch))
			return false;

		std::vector<unsigned char> areas(indices.size() / 3, RC_WALKABLE_AREA);
		rcRasterizeTriangles(&ctx, verts.data(), (int)vertices.size(),
			(const int *)indices.data(), areas.data(), (int)(indices.size() / 3),
			*heightfield, config.walkableClimb);

		rcFilterLowHangingWalkableObstacles(&ctx, config.walkableClimb, *heightfield);
		rcFilterLedgeSpans(&ctx, config.walkableHeight, config.walkableClimb, *heightfield);
		rcFilterWalkableLowHeightSpans(&ctx, config.walkableHeight, *heightfield);

		compactHeightfield = rcAllocCompactHeightfield();
		if (!compactHeightfield) return false;
		if (!rcBuildCompactHeightfield(&ctx, config.walkableHeight, config.walkableClimb, *heightfield, *compactHeightfield))
			return false;

		rcErodeWalkableArea(&ctx, config.walkableRadius, *compactHeightfield);

		contourSet = rcAllocContourSet();
		if (!contourSet) return false;
		if (!rcBuildContours(&ctx, *compactHeightfield, config.maxSimplificationError, config.maxEdgeLen, *contourSet))
			return false;

		polyMesh = rcAllocPolyMesh();
		if (!polyMesh) return false;
		if (!rcBuildPolyMesh(&ctx, *contourSet, config.maxVertsPerPoly, *polyMesh))
			return false;

		rcPolyMeshDetail *detailMesh = nullptr;
		detailMesh = rcAllocPolyMeshDetail();
		rcBuildPolyMeshDetail(&ctx, *polyMesh, *compactHeightfield, config.detailSampleDist, config.detailSampleMaxError, *detailMesh);

		// Fill dtNavMeshCreateParams
		dtNavMeshCreateParams params{};
		memset(&params, 0, sizeof(params));
		params.verts = polyMesh->verts; 
		params.vertCount = (int)vertices.size();
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
		params.walkableHeight = config.walkableHeight;
		params.walkableRadius = config.walkableRadius;
		params.walkableClimb = config.walkableClimb;
		params.cs = config.cs;
		params.ch = config.ch;
		params.buildBvTree = true;

		unsigned char *navData = nullptr;
		int navDataSize = 0;

		if (!dtCreateNavMeshData(&params, &navData, &navDataSize)) return false;

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
