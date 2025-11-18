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
#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>
#include <DetourNavMeshQuery.h>
#include <DetourCommon.h>
#include "Core/Core.h"
#include <Navigation/NavigationSystem.h>
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
		config.cs = 0.2f;
		config.ch = 0.01f;
		config.walkableHeight = (int)ceilf(2.0f / config.ch);
		config.walkableClimb = (int)floorf(0.5f / config.ch);
		config.walkableRadius = (int)ceilf(0.4f / config.cs);
		config.maxEdgeLen = (int)(12.0f / config.cs);
		config.maxSimplificationError = 1.3f;
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
	}

	rcConfig& RecastNavMesh::GetConfig()
	{
		return config;
	}

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

		//rcErodeWalkableArea(&ctx, config.walkableRadius, *compactHeightfield);

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

		std::ofstream objFile("Resources/navmesh_debug.obj");
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
			std::cout << "NavMesh exported to navmesh_debug.obj (" << detailMesh->nverts
				<< " verts, " << detailMesh->ntris << " tris)" << std::endl;
		}
		else
		{
			std::cout << "Failed to write navmesh_debug.obj" << std::endl;
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
		std::ofstream outFile("Resources/output_navmesh.bin", std::ios::binary);
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

		SliceEngine::NavMeshObj obj{ navMesh, navQuery };
		SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::NavigationSystem>().LoadNavMeshFromBake(obj);

		return true;
	}

	// its 256b 
	// maybe i adjust this to be model*
	bool RecastNavMesh::BuildFromModel(const std::vector<SliceEngine::SliceEngineTypes::Model*> models, const std::vector<glm::mat4> &transform)
	{
		if (models.size() != transform.size())
		{
			std::cerr << "ERROR: Model and transform count mismatch in RecastNavMesh::BuildFromModel\n";
			return false;
		}

		Clear();

		std::vector<SliceEngine::SliceEngineTypes::Vertex> vertices;
		std::vector<unsigned int> indices;

		size_t vertexOffset = 0;
		for (size_t i = 0; i < models.size(); ++i)
		{
			const auto &mdl = *models[i];
			const auto &baseTransform = transform[i];

			CollectMeshDataFromNode(mdl, mdl.rootNode, baseTransform, vertices, indices, vertexOffset);
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

		if (!rcBuildDistanceField(&ctx, *compactHeightfield))
			return false;

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

		std::ofstream objFile("Resources/navmesh_debug.obj");
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
			std::cout << "NavMesh exported to navmesh_debug.obj (" << detailMesh->nverts
				<< " verts, " << detailMesh->ntris << " tris)" << std::endl;
		}
		else
		{
			std::cout << "Failed to write navmesh_debug.obj" << std::endl;
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
		rcVcopy(params.bmin, polyMesh->bmin);
		rcVcopy(params.bmax, polyMesh->bmax);

		params.cs = config.cs;
		params.ch = config.ch;
		params.buildBvTree = true;

		unsigned char *navData = nullptr;
		int navDataSize = 0;
		if (!dtCreateNavMeshData(&params, &navData, &navDataSize)) return false;

		// testing if can save into file, this is for detour to read
		std::ofstream outFile("Resources/output_navmesh.bin", std::ios::binary);
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

		SliceEngine::NavMeshObj obj{ navMesh, navQuery };
		SliceEngine::Core::GetInstance()->GetSystem<SliceEngine::NavigationSystem>().LoadNavMeshFromBake(obj);

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
