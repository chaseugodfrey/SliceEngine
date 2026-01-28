/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        RecastNavMesh.cpp

 author:	  Crystal Koh Qiao Wei

 email:       k.crystalqiaowei@digipen.edu

 brief:		  Responsible for initialising and updating the RecastNavMesh

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "RecastNavMesh.h"
#include <cstring>
#include <filesystem>
#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>
#include <DetourNavMeshQuery.h>
#include <DetourCommon.h>
#include "Core/Core.h"
#include <Navigation/NavigationSystem.h>
#include <Systems/SceneSystem.h>
#include <ECS/ECSTypes.h>

namespace SliceEditor
{
	RecastNavMesh::RecastNavMesh() {}

	RecastNavMesh::~RecastNavMesh()
	{
		Clear();
	}

	void RecastNavMesh::Init()
	{
		memset(&config, 0, sizeof(config));
		m_agentHeight = 2.0f;
		m_agentRadius = 0.f;
		m_agentMaxClimb = 0.5f;
		config.cs = 0.1f;
		config.ch = 0.01f;
		config.walkableSlopeAngle = 45.0f;
		config.walkableHeight = (int)ceilf(m_agentHeight / config.ch); // Agent Height
		config.walkableClimb = (int)floorf(m_agentMaxClimb / config.ch); // Max Climb
		config.walkableRadius = (int)ceilf(m_agentRadius / config.cs); // Agent Radius
		config.maxEdgeLen = (int)(12.0f / config.cs);
		config.maxSimplificationError = 1.1f;
		config.minRegionArea = (int)rcSqr(8);
		config.mergeRegionArea = (int)rcSqr(20);
		config.maxVertsPerPoly = 6;
		config.detailSampleDist = config.cs * 6.0f;
		config.detailSampleMaxError = config.ch * 1.0f;
	}

	void RecastNavMesh::Clear()
	{
		if (navQuery) dtFreeNavMeshQuery(navQuery);
		if (navMesh) dtFreeNavMesh(navMesh);

		if (polyMesh) rcFreePolyMesh(polyMesh);
		if (contourSet) rcFreeContourSet(contourSet);
		if (compactHeightfield) rcFreeCompactHeightfield(compactHeightfield);
		if (heightfield) rcFreeHeightField(heightfield);

		if (detailMesh) rcFreePolyMeshDetail(detailMesh);

		navQuery = nullptr;
		navMesh = nullptr;
		polyMesh = nullptr;
		contourSet = nullptr;
		compactHeightfield = nullptr;
		heightfield = nullptr;

		detailMesh = nullptr;

		SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::NavigationSystem>().ClearNavMesh();

	}

	rcConfig& RecastNavMesh::GetConfig()
	{
		return config;
	}

	float *RecastNavMesh::GetAgentHeight()
	{
		return &m_agentHeight;
	}

	float *RecastNavMesh::GetAgentRadius()
	{
		return &m_agentRadius;
	}

	float *RecastNavMesh::GetMaxClimb()
	{
		return &m_agentMaxClimb;
	}

	// SINGULAR MODEL
	bool RecastNavMesh::BuildFromModel(const SliceEngine::SliceEngineTypes::Model &model, const glm::mat4 &transform)
	{
		Clear();

		std::vector<SliceEngine::SliceEngineTypes::Vertex> vertices;
		std::vector<unsigned int> indices;

		for (const auto &mesh : model.meshes)
		{
			for (const auto &v : mesh.vertices)
			vertices.push_back(v);

			for (const auto &ind : mesh.indices)
				indices.push_back(ind);
		}

		if (vertices.empty() || indices.empty())
			return false;

		std::vector<float> verts;
		verts.reserve(vertices.size() * 3);

		for (size_t i = 0; i < vertices.size(); ++i)
		{
			const auto &v = vertices[i].position;

			glm::vec4 worldPos = transform * glm::vec4(v.x, v.y, v.z, 1.0f);

			verts.push_back(worldPos.x);
			verts.push_back(worldPos.y);  
			verts.push_back(worldPos.z);  
		}

		//std::cout << "Input vertex count: " << vertices.size() << std::endl;
		//for (auto &v : verts)
		//	std::cout << "(" << v << ")\n";
		//std::cout << "indices.size(): " << indices.size() << std::endl;
		//std::cout << "verts.size(): " << verts.size() << std::endl;

		std::vector<int> recastIndices(indices.begin(), indices.end());

		float bmin[3], bmax[3];
		rcCalcBounds(verts.data(), (int)(verts.size() / 3), bmin, bmax);

		//std::cout << "Bounds: (" << bmin[0] << "," << bmin[1] << "," << bmin[2] << ") to ("
		//	<< bmax[0] << "," << bmax[1] << "," << bmax[2] << ")" << std::endl;

		if (bmax[1] - bmin[1] < 0.01f)
		{
			bmax[1] = bmin[1] + 0.01f;  
		}

		rcCalcGridSize(bmin, bmax, config.cs, &config.width, &config.height);

		rcContext ctx;
		heightfield = rcAllocHeightfield();
		if (!heightfield) return false;

		if (!rcCreateHeightfield(&ctx, *heightfield, config.width, config.height, bmin, bmax, config.cs, config.ch))
			return false;

		std::vector<unsigned char> areas(recastIndices.size() / 3, RC_WALKABLE_AREA);


		rcRasterizeTriangles(&ctx, verts.data(), (int)verts.size() / 3,
			recastIndices.data(), areas.data(), (int)(recastIndices.size() / 3),
			*heightfield, config.walkableClimb);

		//std::cout << "Heightfield width: " << heightfield->width
		//	<< " height: " << heightfield->height << std::endl;
		int spanCount = 0;
		for (int i = 0; i < heightfield->width * heightfield->height; ++i)
		{
			for (rcSpan *s = heightfield->spans[i]; s; s = s->next)
			{
				spanCount++;
			}
		}
		//std::cout << "Heightfield span count: " << spanCount << std::endl;

		//rcFilterLowHangingWalkableObstacles(&ctx, config.walkableClimb, *heightfield);
		//rcFilterLedgeSpans(&ctx, config.walkableHeight, config.walkableClimb, *heightfield);
		//rcFilterWalkableLowHeightSpans(&ctx, config.walkableHeight, *heightfield);

		compactHeightfield = rcAllocCompactHeightfield();
		if (!compactHeightfield) return false;
		if (!rcBuildCompactHeightfield(&ctx, config.walkableHeight, config.walkableClimb, *heightfield, *compactHeightfield))
			return false;
		
		//std::cout << "Compact heightfield span count: " << compactHeightfield->spanCount << std::endl;
		//std::cout << "Compact heightfield max region count: " << compactHeightfield->maxRegions << std::endl;
		if (!rcBuildDistanceField(&ctx, *compactHeightfield))
			return false;

		if (!rcBuildRegions(&ctx, *compactHeightfield, 0, config.minRegionArea, config.mergeRegionArea))
			return false;

		//std::cout << "Max region: " << compactHeightfield->maxRegions << std::endl;

		if (compactHeightfield->maxRegions == 0)
		{
			std::cout << "ERROR: No regions were created!" << std::endl;
			return false;
		}

		rcErodeWalkableArea(&ctx, config.walkableRadius, *compactHeightfield);

		contourSet = rcAllocContourSet();
		if (!contourSet) return false;
		if (!rcBuildContours(&ctx, *compactHeightfield, config.maxSimplificationError, config.maxEdgeLen, *contourSet))
			return false;
		//std::cout << "Contour count: " << contourSet->nconts << std::endl;


		polyMesh = rcAllocPolyMesh();
		if (!polyMesh) return false;
		if (!rcBuildPolyMesh(&ctx, *contourSet, config.maxVertsPerPoly, *polyMesh))
			return false;
		//std::cout << "PolyMesh nverts: " << polyMesh->nverts << " npolys: " << polyMesh->npolys << std::endl;
		
		detailMesh = rcAllocPolyMeshDetail();
		rcBuildPolyMeshDetail(&ctx, *polyMesh, *compactHeightfield, config.detailSampleDist, config.detailSampleMaxError, *detailMesh);

		std::string currentSceneName = SliceEngine::Core::GetInstance()->GetSceneSystem()->GetCurrentSceneName();
		std::string debugPath = "Assets/NavMesh/" + currentSceneName + ".navmesh";
		std::filesystem::path path(debugPath);
		if (!std::filesystem::exists(path.parent_path()))
		{
			std::filesystem::create_directories(path.parent_path());
		}

		std::ofstream objFile(debugPath);
		//std::ofstream objFile("Resources/navmesh_debug.obj");
		if (objFile.is_open())
		{
			for (int i = 0; i < detailMesh->nverts; ++i)
			{
				const float *v = &detailMesh->verts[i * 3];
				objFile << "v " << v[0] << " " << v[1] << " " << v[2] << "\n";
			}

			for (int i = 0; i < detailMesh->ntris; ++i)
			{
				const unsigned char *t = &detailMesh->tris[i * 4];
				objFile << "f "
					<< (int)t[0] + 1 << " "
					<< (int)t[1] + 1 << " "
					<< (int)t[2] + 1 << "\n";
			}

			objFile.close();
			std::cout << "NavMesh exported to " << debugPath << " (" << detailMesh->nverts
				<< " verts, " << detailMesh->ntris << " tris)" << std::endl;
			/*std::cout << "NavMesh exported to navmesh_debug.obj (" << detailMesh->nverts
				<< " verts, " << detailMesh->ntris << " tris)" << std::endl;*/
		}
		else
		{
			std::cout << "Failed to write " << debugPath << std::endl;
			//std::cout << "Failed to write navmesh_debug.obj" << std::endl;
		}

		for (int i = 0; i < polyMesh->npolys; ++i)
		{
			if (polyMesh->areas[i] == RC_WALKABLE_AREA)
			{
				polyMesh->flags[i] = 1;
			}
		}

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
		//std::cout << "verts: " << params.vertCount
		//	<< " polys: " << params.polyCount
		//	<< " nvp: " << params.nvp
		//	<< " detailVerts: " << params.detailVertsCount
		//	<< " detailTris: " << params.detailTriCount
		//	<< std::endl;
		if (!dtCreateNavMeshData(&params, &navData, &navDataSize)) return false;

		// testing if can save into file, this is for detour to read
		std::ofstream outFile("Resources/" + currentSceneName + ".bin", std::ios::binary);
		outFile.write(reinterpret_cast<const char *>(navData), navDataSize);
		outFile.close();

		navMesh = dtAllocNavMesh();
		if (dtStatusFailed(navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA)))
		{
			dtFree(navData);
			return false;
		}

		navQuery = dtAllocNavMeshQuery();
		navQuery->init(navMesh, 2048);

		dtCrowd *crowd = SliceEngine::NavMeshUtilities::InitCrowd(navMesh);

		SliceEngine::NavMeshObj obj{ navMesh, navQuery,crowd };
		SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::NavigationSystem>().LoadNavMeshFromBake(obj);

		navMesh = nullptr;
		navQuery = nullptr;

		return true;
	}

	// its 256b 
	// maybe i adjust this to be model*
	bool RecastNavMesh::BuildFromModel(const std::vector<SliceEngine::SliceEngineTypes::Model*> models, const std::vector<glm::mat4> &transform,
		const std::vector<SliceEngine::NavMeshLink> &links)
	{
		if (models.size() != transform.size())
		{
			std::cerr << "ERROR: Model and transform count mismatch in RecastNavMesh::BuildFromModel\n";
			return false;
		}

		Clear();

		std::vector<SliceEngine::SliceEngineTypes::Vertex> vertices;
		std::vector<unsigned int> indices;

		std::vector<size_t> modelIndexEndPoints;

		size_t vertexOffset = 0;
		for (size_t i = 0; i < models.size(); ++i)
		{
			const auto &mdl = *models[i];
			glm::mat4 baseTransform = transform[i];

			// Optional: Lift obstacles slightly (e.g., 0.1f) if you still see merging issues
			// if (i > 0) baseTransform = glm::translate(baseTransform, glm::vec3(0.0f, 0.1f, 0.0f));
			if (i > 0)
			{
				baseTransform = glm::translate(baseTransform, glm::vec3(0.0f, 0.2f, 0.0f));
			}
			CollectMeshDataFromNode(mdl, mdl.rootNode, baseTransform, vertices, indices, vertexOffset);

			// --- RESTORED: Save index count ---
			modelIndexEndPoints.push_back(indices.size());
		}

		if (vertices.empty() || indices.empty())
			return false;

		std::vector<float> verts;
		verts.reserve(vertices.size() * 3);
		for (const auto &v : vertices)
		{
			verts.push_back(v.position.x);
			verts.push_back(v.position.y);
			verts.push_back(v.position.z);
		}

		std::vector<int> recastIndices(indices.begin(), indices.end());

		float bmin[3], bmax[3];
		rcCalcBounds(verts.data(), (int)(verts.size() / 3), bmin, bmax);


		if (bmax[1] - bmin[1] < 5.0f)
		{
			bmax[1] = bmin[1] + 5.0f;
		}

		rcCalcGridSize(bmin, bmax, config.cs, &config.width, &config.height);

		rcContext ctx;
		heightfield = rcAllocHeightfield();
		if (!heightfield) return false;

		if (!rcCreateHeightfield(&ctx, *heightfield, config.width, config.height, bmin, bmax, config.cs, config.ch))
			return false;

		std::vector<unsigned char> areas(recastIndices.size() / 3, RC_WALKABLE_AREA);


		// Mark based on slope (this marks the floor AND the top of the wall as walkable)
		rcMarkWalkableTriangles(&ctx, config.walkableSlopeAngle,
			verts.data(), (int)verts.size() / 3,
			recastIndices.data(), (int)recastIndices.size() / 3,
			areas.data());

		// --- SMART SLOPE LOGIC ---
				// Instead of blindly blocking Model 1+, we check the normal.
		size_t currentIndexStart = 0;
		for (size_t i = 0; i < models.size(); ++i)
		{
			size_t currentIndexEnd = modelIndexEndPoints[i];

			// For secondary models (Walls/Ramps)
			if (i > 0)
			{
				size_t startTriIndex = currentIndexStart / 3;
				size_t endTriIndex = currentIndexEnd / 3;

				for (size_t t = startTriIndex; t < endTriIndex; ++t)
				{
					// Calculate Triangle Normal
					int v0_idx = recastIndices[t * 3 + 0];
					int v1_idx = recastIndices[t * 3 + 1];
					int v2_idx = recastIndices[t * 3 + 2];

					const float *v0 = &verts[v0_idx * 3];
					const float *v1 = &verts[v1_idx * 3];
					const float *v2 = &verts[v2_idx * 3];

					float e0[3], e1[3], normal[3];
					rcVsub(e0, v1, v0);
					rcVsub(e1, v2, v0);
					rcVcross(normal, e0, e1);
					rcVnormalize(normal);

					// Calculate Slope Angle (Angle between Normal and Up-Vector Y)
					// Dot product of Normal and (0, 1, 0) is just normal[1]
					float slopeCos = normal[1];

					// Threshold for "Wall"
					// If slopeCos is close to 0, it's a vertical wall (Normal is horizontal).
					// If slopeCos is close to 1, it's flat ground.
					// cos(45) ~= 0.707. 
					// So if normal.y < 0.707, it is steeper than 45 degrees.

					float walkableThr = cosf(config.walkableSlopeAngle / 180.0f * RC_PI);

					// If it is steeper than our limit, mark as NULL (Obstacle)
					// OTHERWISE, leave it as WALKABLE (so ramps work!)
					if (slopeCos < walkableThr)
					{
						if (t < areas.size()) areas[t] = RC_NULL_AREA;
					}
					else
					{
						// It's a walkable slope!
						// Ensure we don't accidentally overwrite it if it was already marked walkable
						if (t < areas.size()) areas[t] = RC_WALKABLE_AREA;
					}
				}
			}
			currentIndexStart = currentIndexEnd;
		}
		rcRasterizeTriangles(&ctx, verts.data(), (int)verts.size() / 3,
			recastIndices.data(), areas.data(), (int)(recastIndices.size() / 3),
			*heightfield, config.walkableClimb);

		int spanCount = 0;
		for (int i = 0; i < heightfield->width * heightfield->height; ++i)
		{
			for (rcSpan *s = heightfield->spans[i]; s; s = s->next)
			{
				spanCount++;
			}
		}

		//rcFilterLowHangingWalkableObstacles(&ctx, config.walkableClimb, *heightfield);
		//rcFilterLedgeSpans(&ctx, config.walkableHeight, config.walkableClimb, *heightfield);
		//rcFilterWalkableLowHeightSpans(&ctx, config.walkableHeight, *heightfield);

		compactHeightfield = rcAllocCompactHeightfield();
		if (!compactHeightfield) return false;

		if (!rcBuildCompactHeightfield(&ctx, config.walkableHeight, config.walkableClimb, *heightfield, *compactHeightfield))
			return false;

		if (!rcErodeWalkableArea(&ctx, config.walkableRadius, *compactHeightfield))
		{
			std::cerr << "ERROR: Failed to erode walkable area!" << std::endl;
			return false;
		}

		if (!rcBuildDistanceField(&ctx, *compactHeightfield))
			return false;


		std::cout << "[RecastDebug] Compact Span Count: " << compactHeightfield->spanCount << std::endl;

		if (compactHeightfield->spanCount == 0)
		{
			std::cerr << "ERROR: Recast found ZERO walkable spans! Check your winding order or model size." << std::endl;
			return false;
		}


		if (!rcBuildRegions(&ctx, *compactHeightfield, 0, config.minRegionArea, config.mergeRegionArea))
			return false;


		if (compactHeightfield->maxRegions == 0)
		{
			std::cout << "ERROR: No regions were created!" << std::endl;
			return false;
		}

		contourSet = rcAllocContourSet();
		if (!contourSet) return false;
		if (!rcBuildContours(&ctx, *compactHeightfield, config.maxSimplificationError, config.maxEdgeLen, *contourSet))
			return false;


		polyMesh = rcAllocPolyMesh();
		if (!polyMesh) return false;
		if (!rcBuildPolyMesh(&ctx, *contourSet, config.maxVertsPerPoly, *polyMesh))
			return false;

		detailMesh = rcAllocPolyMeshDetail();
		rcBuildPolyMeshDetail(&ctx, *polyMesh, *compactHeightfield, config.detailSampleDist, config.detailSampleMaxError, *detailMesh);

		std::string currentSceneName = SliceEngine::Core::GetInstance()->GetSceneSystem()->GetCurrentSceneName();
		std::string debugPath = "Assets/NavMesh/" + currentSceneName + ".navmesh";
		std::filesystem::path path(debugPath);
		if (!std::filesystem::exists(path.parent_path()))
		{
			std::filesystem::create_directories(path.parent_path());
		}

		std::ofstream objFile(debugPath);
		//std::ofstream objFile("Resources/navmesh_debug.obj");
		if (objFile.is_open())
		{
			for (int i = 0; i < detailMesh->nverts; ++i)
			{
				const float *v = &detailMesh->verts[i * 3];
				objFile << "v " << v[0] << " " << v[1] << " " << v[2] << "\n";
			}

			for (int i = 0; i < detailMesh->ntris; ++i)
			{
				const unsigned char *t = &detailMesh->tris[i * 4];
				objFile << "f "
					<< (int)t[0] + 1 << " "
					<< (int)t[1] + 1 << " "
					<< (int)t[2] + 1 << "\n";
			}

			objFile.close();
			std::cout << "NavMesh exported to " << debugPath << " (" << detailMesh->nverts
				<< " verts, " << detailMesh->ntris << " tris)" << std::endl;
			/*std::cout << "NavMesh exported to navmesh_debug.obj (" << detailMesh->nverts
				<< " verts, " << detailMesh->ntris << " tris)" << std::endl;*/
		}
		else
		{
			std::cout << "Failed to write " << debugPath << std::endl;
			//std::cout << "Failed to write navmesh_debug.obj" << std::endl;
		}

		for (int i = 0; i < polyMesh->npolys; ++i)
		{/*
			if (polyMesh->areas[i] == RC_WALKABLE_AREA)
			{*/
				polyMesh->flags[i] = 1;
			//}
		}

		std::vector<float> offMeshVerts;
		std::vector<float> offMeshRad;
		std::vector<unsigned char> offMeshDir;
		std::vector<unsigned char> offMeshAreas;
		std::vector<unsigned short> offMeshFlags;
		std::vector<unsigned int> offMeshUserID;

		for (size_t i = 0; i < links.size(); ++i)
		{
			const auto &link = links[i];

			// Start
			offMeshVerts.push_back(link.startLink.x);
			offMeshVerts.push_back(link.startLink.y);
			offMeshVerts.push_back(link.startLink.z);

			// End
			offMeshVerts.push_back(link.endLink.x);
			offMeshVerts.push_back(link.endLink.y);
			offMeshVerts.push_back(link.endLink.z);

			offMeshRad.push_back(link.radius);
			offMeshDir.push_back(link.bidirectional ? 1 : 0);
			offMeshAreas.push_back(RC_WALKABLE_AREA); // Standard walkable area
			offMeshFlags.push_back(1);                // Standard walkable flag
			offMeshUserID.push_back((unsigned int)i + 1); // Simple ID

			// In RecastNavMesh.cpp loop
			std::cout << "[Recast] Baking Link " << i << ": Start("
				<< link.startLink.x << "," << link.startLink.y << "," << link.startLink.z << ") -> End("
				<< link.endLink.x << "," << link.endLink.y << "," << link.endLink.z << ")" << std::endl;
		}

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

		params.offMeshConVerts = offMeshVerts.data();
		params.offMeshConRad = offMeshRad.data();
		params.offMeshConDir = offMeshDir.data();
		params.offMeshConAreas = offMeshAreas.data();
		params.offMeshConFlags = offMeshFlags.data();
		params.offMeshConUserID = offMeshUserID.data();
		params.offMeshConCount = (int)offMeshRad.size();

		params.walkableHeight = m_agentHeight;
		params.walkableRadius = m_agentRadius;
		params.walkableClimb = m_agentMaxClimb;
		rcVcopy(params.bmin, polyMesh->bmin);
		rcVcopy(params.bmax, polyMesh->bmax);

		params.cs = config.cs;
		params.ch = config.ch;
		params.buildBvTree = true;

		unsigned char *navData = nullptr;
		int navDataSize = 0;
		std::cout << "[Recast] Attempting to bake " << params.offMeshConCount << " off-mesh connections." << std::endl;
		if (params.offMeshConCount > 0)
		{
			std::cout << "  Link 0 Start: " << params.offMeshConVerts[0] << ", " << params.offMeshConVerts[1] << ", " << params.offMeshConVerts[2] << std::endl;
		}

		if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
		{
			SLICE_LOG_ERROR("Could not build Detour navmesh.");
			return false;
		}

		// testing if can save into file, this is for detour to read
		std::ofstream outFile("Assets/NavMesh/" + currentSceneName + ".bin", std::ios::binary);
		std::cout << "Detour file NavMesh exported to Assets/NavMesh/" << currentSceneName << ".bin\n";

		if (polyMesh)
		{
			int polyCount = polyMesh->npolys;
			std::cout << "NavMesh polygon count: " << polyCount << std::endl;
		}

		outFile.write(reinterpret_cast<const char *>(navData), navDataSize);
		outFile.close();

		navMesh = dtAllocNavMesh();
		if (dtStatusFailed(navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA)))
		{
			dtFree(navData);
			return false;
		}

		navQuery = dtAllocNavMeshQuery();
		navQuery->init(navMesh, 2048);

		dtCrowd *crowd = SliceEngine::NavMeshUtilities::InitCrowd(navMesh);

		SliceEngine::NavMeshObj obj{ navMesh, navQuery,crowd };
		SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::NavigationSystem>().LoadNavMeshFromBake(obj);

		navMesh = nullptr;
		navQuery = nullptr;

		// After building the navmesh, add:
		std::cout << "NavMesh bounds: ("
			<< polyMesh->bmin[0] << ", " << polyMesh->bmin[1] << ", " << polyMesh->bmin[2] << ") to ("
			<< polyMesh->bmax[0] << ", " << polyMesh->bmax[1] << ", " << polyMesh->bmax[2] << ")" << std::endl;
		return true;
	}

	void RecastNavMesh::CollectMeshDataFromNode(
		const SliceEngine::SliceEngineTypes::Model &model,
		const SliceEngine::SliceEngineTypes::ModelNode &node,
		const glm::mat4 &parentTransform,
		std::vector<SliceEngine::SliceEngineTypes::Vertex> &outVertices,
		std::vector<unsigned int> &outIndices,
		size_t &vertexOffset)
	{
		glm::mat4 localTransform =
			parentTransform *
			glm::translate(glm::mat4(1.0f), node.position) *
			glm::mat4_cast(node.rotation) *
			glm::scale(glm::mat4(1.0f), node.scale);

		for (unsigned short meshIndex : node.mesh_ref)
		{
			if (meshIndex >= model.meshes.size()) continue;
			const auto &mesh = model.meshes[meshIndex];

			for (const auto &v : mesh.vertices)
			{
				glm::vec4 worldPos = localTransform * glm::vec4(v.position, 1.0f);
				auto transformed = v;
				transformed.position = glm::vec3(worldPos);
				outVertices.push_back(transformed);
			}

			for (auto idx : mesh.indices)
				outIndices.push_back(idx + static_cast<unsigned int>(vertexOffset));

			vertexOffset += mesh.vertices.size();
		}

		for (const auto &child : node.children)
		{
			CollectMeshDataFromNode(model, child, localTransform, outVertices, outIndices, vertexOffset);
		}
	}
}
