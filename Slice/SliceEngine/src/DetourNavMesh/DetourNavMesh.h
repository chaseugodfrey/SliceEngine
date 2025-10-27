#ifndef DETOUR_NAVMESH_H
#define DETOUR_NAVMESH_H

#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>
#include <DetourCommon.h>
namespace SliceEngine
{
	class DetourNavMesh
	{
	private:
		dtNavMesh *navMesh = nullptr;
		dtNavMeshQuery *navQuery = nullptr;
	};
}
#endif