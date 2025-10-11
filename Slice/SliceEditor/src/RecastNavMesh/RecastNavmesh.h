#ifndef MESH_H
#define MESH_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <Recast.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>
// need detour libs so i can send the data into detour, which can be used in engine side
namespace SliceEditor
{
	struct Vertex
	{
		glm::vec3 position{};
		glm::vec3 normal{};
		glm::vec2 uv{};
	};
	class RecastNavMesh
	{
	public:
		RecastNavMesh();
		~RecastNavMesh();

		bool BuildFromObj(const std::string &objPath, const std::string &outputPath);
	private:
		bool LoadFromObj(const std::string &path, std::vector<float> &outVerts, std::vector<int> &outTris);
		bool BuildNavMesh(std::vector<float> &verts, std::vector<int> &tris, const std::string& outPath);
		rcConfig cfg;
	};
}
#endif