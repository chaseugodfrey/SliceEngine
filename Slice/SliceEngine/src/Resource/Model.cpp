/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Model.cpp
 author:		Elton leosantosa
 email:			leosantosa@digipen.edu
 brief:			Loads Models

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "Model.h"
#include <fstream>
#include <glm/gtc/type_ptr.hpp>

namespace {
	//some consts to help typing
	constexpr uint16_t version_number = 4;	//i think having a vers number could be useful, maybe
	constexpr uint64_t i_size = sizeof(unsigned int);
	constexpr uint64_t f_size = sizeof(float);
	const float PIF = 3.14159265359f;
	const float PI05F = 1.57079632679f;
}

namespace SliceEngine
{
	namespace SliceEngineTypes
	{
		bool Model::LoadModelResource(std::string const& file) {
			std::ifstream fs(file, std::ios::binary);

			if (!fs)
			{
				SLICE_LOG_WARNING("Unable to open Obj:" + file);
				return false;
			}

			/*
			* Checking for valid header
			*/
			constexpr uint64_t header_size = 3 + sizeof(version_number) + sizeof(uint64_t);
			char header_buffer[header_size]{};
			try {
				fs.read(header_buffer, header_size);
			}
			catch (...) {
				SLICE_LOG_WARNING("Error reading file: " + file);
				fs.close();
				return false;
			}

			if (fs.fail() || fs.eof()) {
				SLICE_LOG_WARNING("Unknown file format: " + file);
				fs.close();
				return false;
			}

			if (header_buffer[0] != 'M' || header_buffer[1] != 'D' || header_buffer[2] != 'L') {
				SLICE_LOG_WARNING("Not a proper mdl file: " + file);
				fs.close();
				return false;
			}
			auto vers = version_number;
			vers = *((decltype(version_number)*)(header_buffer + 3));
			if (vers != version_number) {
				SLICE_LOG_WARNING("Wrong version, please recompile: " + file);
				fs.close();
				return false;
			}


			//finally, get the file size
			uint64_t buffer_size = *((uint64_t*)(header_buffer + 3 + sizeof(version_number)));
			char* const buffer = new char[buffer_size];
			uint64_t offset{};

			fs.read(buffer, buffer_size);

			fs.close();

			unpack_data(buffer, offset);
			delete[] buffer;

			return true;
		}

		void Model::unpack_data(char* const buffer, uint64_t& offset) {
			//extract is static to decide
			memcpy(&is_static, buffer + offset, 1); offset += 1;
			uint32_t dest{};
			//name
			memcpy(&dest, buffer + offset, i_size); offset += i_size;
			name.resize(dest);
			memcpy(name.data(), buffer + offset, dest); offset += dest;
			//meshes
			memcpy(&dest, buffer + offset, i_size); offset += i_size;
			meshes.resize(dest);
			if (is_static) {
				for (auto& m : meshes) {
					m.unpack_static_data(buffer, offset);
					m.setup_mesh();
				}
			}
			else {
				for (auto& m : meshes) {
					m.unpack_skin_data(buffer, offset);
					m.setup_mesh();
				}
			}

			//node hierachy
			rootNode.unpack_data(buffer, offset);
		}
		void Mesh::unpack_static_data(char const* const buffer, uint64_t& offset) {
			static_model = true;
			uint32_t dest{};

			//name
			memcpy(&dest, buffer + offset, i_size); offset += i_size;
			name.resize(dest);
			memcpy(name.data(), buffer + offset, dest); offset += dest;

			uint32_t vert_size{}, idx_size{};//, tex_size{};
			uint32_t num_vert{}, num_idx{};//, num_tex{};

			//buffer details
			memcpy(&vert_size, buffer + offset, i_size); offset += i_size;
			memcpy(&num_vert, buffer + offset, i_size); offset += i_size;

			memcpy(&idx_size, buffer + offset, i_size); offset += i_size;
			memcpy(&num_idx, buffer + offset, i_size); offset += i_size;

			//buffer
			uint64_t vert_buffer_size = vert_size * num_vert;
			uint64_t idx_buffer_size = idx_size * num_idx;

			vertices.resize(num_vert);
			indices.resize(num_idx);
			memcpy(vertices.data(), buffer + offset, vert_buffer_size); offset += vert_buffer_size;
			memcpy(indices.data(), buffer + offset, idx_buffer_size); offset += idx_buffer_size;
		}
		void Mesh::unpack_skin_data(char const* const buffer, uint64_t& offset) {
			static_model = false;
			uint32_t dest{};

			//name
			memcpy(&dest, buffer + offset, i_size); offset += i_size;
			name.resize(dest);
			memcpy(name.data(), buffer + offset, dest); offset += dest;

			uint32_t vert_size{}, idx_size{}, vb_size{};
			uint32_t num_vert{}, num_idx{}, num_vb{};

			//buffer details
			memcpy(&vert_size, buffer + offset, i_size); offset += i_size;
			memcpy(&num_vert, buffer + offset, i_size); offset += i_size;

			memcpy(&idx_size, buffer + offset, i_size); offset += i_size;
			memcpy(&num_idx, buffer + offset, i_size); offset += i_size;

			memcpy(&vb_size, buffer + offset, i_size); offset += i_size;
			memcpy(&num_vb, buffer + offset, i_size); offset += i_size;

			//buffer
			uint64_t vert_buffer_size = (uint64_t)vert_size * num_vert;
			uint64_t idx_buffer_size = (uint64_t)idx_size * num_idx;
			uint64_t vert_bone_buffer_size = (uint64_t)vb_size * num_vb;

			vertices.resize(num_vert);
			indices.resize(num_idx);
			vert_bones.resize(num_vb);
			memcpy(vertices.data(), buffer + offset, vert_buffer_size); offset += vert_buffer_size;
			memcpy(indices.data(), buffer + offset, idx_buffer_size); offset += idx_buffer_size;
			memcpy(vert_bones.data(), buffer + offset, vert_bone_buffer_size); offset += vert_bone_buffer_size;
		}

		void ModelNode::unpack_data(char* const buffer, uint64_t& offset) {
			uint32_t dest{};
			//name
			memcpy(&dest, buffer + offset, i_size); offset += i_size;
			name.resize(dest);
			memcpy(name.data(), buffer + offset, dest); offset += dest;
			//mesh refs
			memcpy(&dest, buffer + offset, i_size); offset += i_size;
			mesh_ref.resize(dest);
			dest *= sizeof(unsigned short);
			memcpy(mesh_ref.data(), buffer + offset, dest); offset += dest;

			//transform
			memcpy(glm::value_ptr(position), buffer + offset, sizeof(glm::vec3)); offset += sizeof(glm::vec3);
			memcpy(glm::value_ptr(rotation), buffer + offset, sizeof(glm::quat)); offset += sizeof(glm::quat);
			memcpy(glm::value_ptr(scale), buffer + offset, sizeof(glm::vec3)); offset += sizeof(glm::vec3);

			//children
			memcpy(&dest, buffer + offset, i_size); offset += i_size;
			children.resize(dest);
			for (auto& child : children) {
				child.unpack_data(buffer, offset);
			}
		}

#pragma region Load Primitive
		void Model::LoadDefaultCubeModel()
		{
			meshes.resize(1);
			auto& mesh = meshes[0];
			auto& vertices = mesh.vertices;	vertices.clear();
			vertices.reserve(24);
			//----------bot
			//left-bot-back     0-2
			vertices.emplace_back(Vertex{ {-0.5f,-0.5f,-0.5f},{-1.f,0.f,0.f},{0.f,1.f} });
			vertices.emplace_back(Vertex{ {-0.5f,-0.5f,-0.5f},{0.f,-1.f,0.f},{0.f,1.f} });
			vertices.emplace_back(Vertex{ {-0.5f,-0.5f,-0.5f},{0.f,0.f,-1.f},{1.f,1.f} });

			//right-bot-back    3-5
			vertices.emplace_back(Vertex{ {0.5f,-0.5f,-0.5f},{1.f,0.f,0.f},{1.f,1.f} });
			vertices.emplace_back(Vertex{ {0.5f,-0.5f,-0.5f},{0.f,-1.f,0.f},{1.f,1.f} });
			vertices.emplace_back(Vertex{ {0.5f,-0.5f,-0.5f},{0.f,0.f,-1.f},{0.f,1.f} });

			//left-bot-front    6-8
			vertices.emplace_back(Vertex{ {-0.5f,-0.5f,0.5f},{-1.f,0.f,0.f},{1.f,1.f} });
			vertices.emplace_back(Vertex{ {-0.5f,-0.5f,0.5f},{0.f,-1.f,0.f},{0.f,0.f} });
			vertices.emplace_back(Vertex{ {-0.5f,-0.5f,0.5f},{0.f,0.f,1.f},{0.f,1.f} });

			//right-bot-front   9-11
			vertices.emplace_back(Vertex{ {0.5f,-0.5f,0.5f},{1.f,0.f,0.f},{0.f,1.f} });
			vertices.emplace_back(Vertex{ {0.5f,-0.5f,0.5f},{0.f,-1.f,0.f},{1.f,0.f} });
			vertices.emplace_back(Vertex{ {0.5f,-0.5f,0.5f},{0.f,0.f,1.f},{1.f,1.f} });

			//----------top
			//left-top-back     12-14
			vertices.emplace_back(Vertex{ {-0.5f,0.5f,-0.5f},{-1.f,0.f,0.f},{0.f,0.f} });
			vertices.emplace_back(Vertex{ {-0.5f,0.5f,-0.5f},{0.f,1.f,0.f},{0.f,0.f} });
			vertices.emplace_back(Vertex{ {-0.5f,0.5f,-0.5f},{0.f,0.f,-1.f},{1.f,0.f} });

			//right-top-back    15-17
			vertices.emplace_back(Vertex{ {0.5f,0.5f,-0.5f},{1.f,0.f,0.f},{1.f,0.f} });
			vertices.emplace_back(Vertex{ {0.5f,0.5f,-0.5f},{0.f,1.f,0.f},{1.f,0.f} });
			vertices.emplace_back(Vertex{ {0.5f,0.5f,-0.5f},{0.f,0.f,-1.f},{0.f,0.f} });

			//left-top-front    18-20
			vertices.emplace_back(Vertex{ {-0.5f,0.5f,0.5f},{-1.f,0.f,0.f},{1.f,0.f} });
			vertices.emplace_back(Vertex{ {-0.5f,0.5f,0.5f},{0.f,1.f,0.f},{0.f,1.f} });
			vertices.emplace_back(Vertex{ {-0.5f,0.5f,0.5f},{0.f,0.f,1.f},{0.f,0.f} });

			//right-top-front   21-23
			vertices.emplace_back(Vertex{ {0.5f,0.5f,0.5f},{1.f,0.f,0.f},{0.f,0.f} });
			vertices.emplace_back(Vertex{ {0.5f,0.5f,0.5f},{0.f,1.f,0.f},{1.f,1.f} });
			vertices.emplace_back(Vertex{ {0.5f,0.5f,0.5f},{0.f,0.f,1.f},{1.f,0.f} });

			auto& indices = mesh.indices;	indices.clear();
			indices.reserve(36);
			/*
				front, left, right, top, bot, back
			*/
			//front
			indices.emplace_back(20); indices.emplace_back(8); indices.emplace_back(11);
			indices.emplace_back(11); indices.emplace_back(23); indices.emplace_back(20);

			//left
			indices.emplace_back(12); indices.emplace_back(0); indices.emplace_back(6);
			indices.emplace_back(6); indices.emplace_back(18); indices.emplace_back(12);

			//right
			indices.emplace_back(21); indices.emplace_back(9); indices.emplace_back(3);
			indices.emplace_back(3); indices.emplace_back(15); indices.emplace_back(21);

			//top
			indices.emplace_back(13); indices.emplace_back(19); indices.emplace_back(22);
			indices.emplace_back(22); indices.emplace_back(16); indices.emplace_back(13);

			//bot
			indices.emplace_back(7); indices.emplace_back(1); indices.emplace_back(4);
			indices.emplace_back(4); indices.emplace_back(10); indices.emplace_back(7);

			//back
			indices.emplace_back(17); indices.emplace_back(5); indices.emplace_back(2);
			indices.emplace_back(2); indices.emplace_back(14); indices.emplace_back(17);

			mesh.setup_mesh();


			//rootNode.local_transform = glm::identity<glm::mat4>();
			rootNode.mesh_ref.resize(1);
			rootNode.mesh_ref[0] = 0;
			rootNode.children.clear();

			name = "Cube";
			return;
		}

		void Model::LoadDefaultSphereModel(int stackCount, int sectorCount)
		{
			meshes.resize(1);
			auto& mesh = meshes[0];
			auto& vertices = mesh.vertices;	vertices.clear();
			vertices.reserve(24);
			auto& indices = mesh.indices;	indices.clear();
			indices.reserve(36);

			float radius = 0.5f;
			vertices.reserve((stackCount + 1) * (sectorCount + 1));
			indices.reserve(stackCount * sectorCount * 6);
			for (int i = 0; i <= stackCount; ++i)
			{
				float v = static_cast<float>(i) / stackCount; // Texture V-coordinate
				float phi = v * PIF; // Latitude angle (0 to PI)

				for (int j = 0; j <= sectorCount; ++j)
				{
					float u = static_cast<float>(j) / sectorCount; // Texture U-coordinate
					float theta = u * 2.0f * PIF; // Longitude angle (0 to 2*PI)

					// Calculate vertex position (Cartesian coordinates from spherical)
					float x = radius * std::sin(phi) * std::cos(theta);
					float y = radius * std::cos(phi);
					float z = radius * std::sin(phi) * std::sin(theta);

					// Calculate normal
					// For a perfect sphere centered at (0,0,0), 
					// the normal is just the normalized position.
					float nx = x / radius;
					float ny = y / radius;
					float nz = z / radius;

					// Add the vertex
					vertices.emplace_back(Vertex{ {x, y, z}, {nx, ny, nz}, {u, v} });
				}
			}
			for (int i = 0; i < stackCount; ++i)
			{
				for (int j = 0; j < sectorCount; ++j)
				{
					// Calculate the indices for the four corners of the current quad
					//
					// (i, j) --- (i, j+1)
					//   |           |
					// (i+1, j) -- (i+1, j+1)
					//
					int first = (i * (sectorCount + 1)) + j;
					int second = first + (sectorCount + 1);

					// First triangle (CCW winding)
					indices.emplace_back(first);
					indices.emplace_back(first + 1);
					indices.emplace_back(second);

					// Second triangle (CCW winding)
					indices.emplace_back(second);
					indices.emplace_back(first + 1);
					indices.emplace_back(second + 1);
				}
			}

			mesh.setup_mesh();

			//rootNode.local_transform = glm::identity<glm::mat4>();
			rootNode.mesh_ref.resize(1);
			rootNode.mesh_ref[0] = 0;
			rootNode.children.clear();
			name = "Sphere";
			return;
		}

		void Model::LoadDefaultCapsuleModel()
		{
			meshes.resize(1);
			auto& mesh = meshes[0];
			auto& vertices = mesh.vertices;	vertices.clear();
			auto& indices = mesh.indices;	indices.clear();

			float radius = 0.5f;
			float halfHeight = 0.5f;    // Half the height of the *cylinder* part
			int capStackCount = 10;     // Stacks for *one* hemisphere cap
			int cylStackCount = 5;      // Stacks for the cylinder body
			int sectorCount = 30;       // Vertical slices (longitude), same as sphere

			// Total stacks for indexing
			int totalStacks = capStackCount + cylStackCount + capStackCount;

			vertices.reserve((totalStacks + 1) * (sectorCount + 1));
			indices.reserve(totalStacks * sectorCount * 6);

			// 2. Generate Vertices
			// We loop from i = 0 to totalStacks (inclusive)
			// This creates (totalStacks + 1) rings of vertices
			for (int i = 0; i <= totalStacks; ++i)
			{
				float phi;
				float y;
				float nx, ny, nz;

				// v-coordinate for texture mapping
				float v = static_cast<float>(i) / totalStacks;

				// We are in the TOP CAP
				if (i <= capStackCount)
				{
					float t = static_cast<float>(i) / capStackCount;
					phi = t * PI05F; // 0 to pi/2

					y = radius * std::cos(phi) + halfHeight; // y-position with offset
					ny = std::cos(phi);
				}
				// We are in the CYLINDER BODY
				else if (i > capStackCount && i <= capStackCount + cylStackCount)
				{
					float t = static_cast<float>(i - capStackCount) / cylStackCount;
					phi = PI05F; // pi/2 (equator)

					y = halfHeight - (t * (halfHeight * 2.0f)); // Interpolate from +h to -h
					ny = 0; // Normal is horizontal
				}
				// We are in the BOTTOM CAP
				else
				{
					float t = static_cast<float>(i - (capStackCount + cylStackCount)) / capStackCount;
					phi = PI05F + (t * PI05F); // pi/2 to pi

					y = radius * std::cos(phi) - halfHeight; // y-position with offset
					ny = std::cos(phi);
				}

				// Inner loop for sectors (longitude)
				for (int j = 0; j <= sectorCount; ++j)
				{
					float u = static_cast<float>(j) / sectorCount;
					float theta = u * 2.0f * PIF;

					//float cosPhi = std::cos(phi); if there is an come here rain
					float sinPhi = std::sin(phi);
					float cosTheta = std::cos(theta);
					float sinTheta = std::sin(theta);

					// Position
					float x = radius * sinPhi * cosTheta;
					float z = radius * sinPhi * sinTheta;

					// Normal (re-calculate x and z components)
					nx = sinPhi * cosTheta;
					nz = sinPhi * sinTheta;

					vertices.emplace_back(Vertex{ {x, y, z}, {nx, ny, nz}, {u, v} });
				}
			}

			// 4. Generate Indices
			// This logic is IDENTICAL to the sphere, as we've built the vertices
			// in a consistent grid.
			for (int i = 0; i < totalStacks; ++i)
			{
				for (int j = 0; j < sectorCount; ++j)
				{
					// Get the indices for the four corners of the quad
					int first = (i * (sectorCount + 1)) + j;
					int second = first + (sectorCount + 1);

					// First triangle
					indices.emplace_back(first);
					indices.emplace_back(first + 1);
					indices.emplace_back(second);

					// Second triangle
					indices.emplace_back(second);
					indices.emplace_back(first + 1);
					indices.emplace_back(second + 1);
				}
			}

			mesh.setup_mesh();


			//rootNode.local_transform = glm::identity<glm::mat4>();
			rootNode.mesh_ref.resize(1);
			rootNode.mesh_ref[0] = 0;
			rootNode.children.clear();
			name = "Capsule";
			return;
		}

		void Model::LoadDefaultCylinderModel()
		{
			meshes.resize(1);
			auto& mesh = meshes[0];
			auto& vertices = mesh.vertices;	vertices.clear();
			auto& indices = mesh.indices;	indices.clear();

			float radius = 0.5f;
			float halfHeight = 0.5f;    // Half the height of the *cylinder* part
			int cylStackCount = 5;      // Stacks for the cylinder body
			int sectorCount = 30;       // Vertical slices (longitude), same as sphere

			// Total stacks for indexing
			int totalStacks = 1 + cylStackCount + 1;

			vertices.reserve((totalStacks + 1) * (sectorCount + 1));
			indices.reserve(totalStacks * sectorCount * 6);

			// 2. Generate Vertices
			// We loop from i = 0 to totalStacks (inclusive)
			// This creates (totalStacks + 1) rings of vertices
			for (int i = 0; i <= totalStacks; ++i)
			{
				float v = static_cast<float>(i) / totalStacks;

				float y, currentRadius, ny;

				// 1. TOP CAP CENTER (i = 0)
				if (i == 0) {
					y = halfHeight;
					currentRadius = 0.0f; // Converge to center
					ny = 1.0f;            // Facing up
				}
				// 2. TOP RIM (i = 1)
				else if (i == 1) {
					y = halfHeight;
					currentRadius = radius;
					ny = 1.0f;            // Facing up
				}
				// 3. CYLINDER BODY (The tube)
				else if (i > 1 && i <= 1 + cylStackCount - 1) {
					float t = static_cast<float>(i - 1) / cylStackCount;
					y = halfHeight - (t * (halfHeight * 2.0f));
					currentRadius = radius;
					ny = 0.0f;            // Facing sideways
				}
				// 4. BOTTOM RIM
				else if (i == totalStacks - 1) {
					y = -halfHeight;
					currentRadius = radius;
					ny = -1.0f;           // Facing down
				}
				// 5. BOTTOM CAP CENTER (i = totalStacks)
				else {
					y = -halfHeight;
					currentRadius = 0.0f; // Converge to center
					ny = -1.0f;           // Facing down
				}

				for (int j = 0; j <= sectorCount; ++j)
				{
					float u = static_cast<float>(j) / sectorCount;
					float theta = u * 2.0f * PIF;

					float cosTheta = std::cos(theta);
					float sinTheta = std::sin(theta);

					float x = currentRadius * cosTheta;
					float z = currentRadius * sinTheta;

					// Normal handling: 
					// If it's a cap, normal is {0, ny, 0}. If it's the body, normal is {cos, 0, sin}
					float nx = (ny == 0.0f) ? cosTheta : 0.0f;
					float nz = (ny == 0.0f) ? sinTheta : 0.0f;

					vertices.emplace_back(Vertex{ {x, y, z}, {nx, ny, nz}, {u, v} });
				}
			}

			// --- Generate Indices ---
			// The logic remains identical to your sphere/capsule code!
			for (int i = 0; i < totalStacks; ++i)
			{
				for (int j = 0; j < sectorCount; ++j)
				{
					int first = (i * (sectorCount + 1)) + j;
					int second = first + (sectorCount + 1);

					indices.emplace_back(first);
					indices.emplace_back(first + 1);
					indices.emplace_back(second);

					indices.emplace_back(second);
					indices.emplace_back(first + 1);
					indices.emplace_back(second + 1);
				}
			}

			mesh.setup_mesh();

			//rootNode.local_transform = glm::identity<glm::mat4>();
			rootNode.mesh_ref.resize(1);
			rootNode.mesh_ref[0] = 0;
			rootNode.children.clear();
			name = "Cylinder";
			return;
		}

		void Model::LoadDefaultQuadModel()
		{
			meshes.resize(1);
			auto& mesh = meshes[0];
			auto& vertices = mesh.vertices;	vertices.clear();
			vertices.reserve(4);
			vertices.emplace_back(Vertex{{-0.5, -0.5, 0.0}, {0.0, 0.0, 1.0}, {0.0, 1.0}});	//bot left
			vertices.emplace_back(Vertex{{ 0.5, -0.5, 0.0}, {0.0, 0.0, 1.0}, {1.0, 1.0}});	//bot right
			vertices.emplace_back(Vertex{{-0.5,  0.5, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0}});	//top left
			vertices.emplace_back(Vertex{{ 0.5,  0.5, 0.0}, {0.0, 0.0, 1.0}, {1.0, 0.0}});	//top right

			auto& indices = mesh.indices;	indices.clear();
			indices.reserve(6);
			indices.emplace_back(0); indices.emplace_back(1); indices.emplace_back(2);
			indices.emplace_back(2); indices.emplace_back(1); indices.emplace_back(3);

			mesh.setup_mesh();

			//rootNode.local_transform = glm::identity<glm::mat4>();
			rootNode.mesh_ref.resize(1);
			rootNode.mesh_ref[0] = 0;
			rootNode.children.clear();
			name = "Quad";
			return;
		}

		void Model::LoadDefaultLineModel()
		{
			meshes.resize(1);
			auto& m = meshes[0];
			m.drawMode = GL_LINES;
			m.drawCnt = 2;
			std::vector<glm::vec3> vtx;
			vtx.reserve(m.drawCnt);
			vtx.emplace_back(-0.5, 0.0, 0.0);
			vtx.emplace_back(0.5, 0.0, 0.0);

			glCreateBuffers(1, &m.vbo);
			glNamedBufferStorage(m.vbo, vtx.size() * sizeof(glm::vec3), vtx.data(), GL_DYNAMIC_STORAGE_BIT);
			glCreateVertexArrays(1, &m.vao);
			// layout=0
			glEnableVertexArrayAttrib(m.vao, 0);
			glVertexArrayVertexBuffer(m.vao, 0, m.vbo, 0, sizeof(glm::vec3));
			glVertexArrayAttribFormat(m.vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
			glVertexArrayAttribBinding(m.vao, 0, 0);

			//rootNode.local_transform = glm::identity<glm::mat4>();
			rootNode.mesh_ref.resize(1);
			rootNode.mesh_ref[0] = 0;
			rootNode.children.clear();
			name = "Line";
			return;
		}

		void Model::LoadDefaultFrustrumModel()
		{
			meshes.resize(1);
			auto& mesh = meshes[0];
			std::vector<glm::vec3> vtx;
			vtx.reserve(8);
			vtx.emplace_back(-0.5f, -0.5f, -0.5f); // 0: Near-Bottom-Left
			vtx.emplace_back(0.5f, -0.5f, -0.5f); // 1: Near-Bottom-Right
			vtx.emplace_back(0.5f, 0.5f, -0.5f); // 2: Near-Top-Right
			vtx.emplace_back(-0.5f, 0.5f, -0.5f); // 3: Near-Top-Left
			// Far face (Z = +0.5)
			vtx.emplace_back(-0.5f, -0.5f, 0.5f); // 4: Far-Bottom-Left
			vtx.emplace_back(0.5f, -0.5f, 0.5f); // 5: Far-Bottom-Right
			vtx.emplace_back(0.5f, 0.5f, 0.5f); // 6: Far-Top-Right
			vtx.emplace_back(-0.5f, 0.5f, 0.5f);  // 7: Far-Top-Left
			
			std::vector<unsigned int> idx;
			idx.reserve(24);
			idx.emplace_back(0); idx.emplace_back(1);
			idx.emplace_back(1); idx.emplace_back(2);
			idx.emplace_back(2); idx.emplace_back(3);
			idx.emplace_back(3); idx.emplace_back(0);

			idx.emplace_back(4); idx.emplace_back(5);
			idx.emplace_back(5); idx.emplace_back(6);
			idx.emplace_back(6); idx.emplace_back(7);
			idx.emplace_back(7); idx.emplace_back(4);

			idx.emplace_back(0); idx.emplace_back(4);
			idx.emplace_back(1); idx.emplace_back(5);
			idx.emplace_back(2); idx.emplace_back(6);
			idx.emplace_back(3); idx.emplace_back(7);

			mesh.setup_mesh();
			//vbo
			glCreateBuffers(1, &mesh.vbo);
			glNamedBufferStorage(mesh.vbo, vtx.size() * sizeof(glm::vec3), vtx.data(), 0);
			//ebo
			glCreateBuffers(1, &mesh.ebo);
			glNamedBufferStorage(mesh.ebo, idx.size() * sizeof(unsigned int), idx.data(), 0);

			//vao
			glCreateVertexArrays(1, &mesh.vao);
			glEnableVertexArrayAttrib(mesh.vao, 0);
			glVertexArrayAttribFormat(mesh.vao, 0, 3, GL_FLOAT, false, 0);
			glVertexArrayVertexBuffer(mesh.vao, 0, mesh.vbo, 0, sizeof(glm::vec3));
			glVertexArrayAttribBinding(mesh.vao, 0, 0);

			glVertexArrayElementBuffer(mesh.vao, mesh.ebo);

			glBindVertexArray(0);
			mesh.drawCnt = static_cast<GLuint>(idx.size());
			mesh.drawMode = GL_LINES;

			//rootNode.local_transform = glm::identity<glm::mat4>();
			rootNode.mesh_ref.resize(1);
			rootNode.mesh_ref[0] = 0;
			rootNode.children.clear();

			name = "Frustrum";
			return;
		}

#pragma endregion

		void Model::DestroyModel() {
			for (auto& m : meshes) {
				if (m.ebo != 0) {
					glDeleteBuffers(1, &m.ebo);
				}
				if (m.static_model) {
					glDeleteBuffers(1, &m.vbbo);
				}
				glDeleteBuffers(1, &m.vbo);
				glDeleteVertexArrays(1, &m.vao);
			}
		}

		void Mesh::setup_mesh() {
			//vbo
			glCreateBuffers(1, &vbo);
			glNamedBufferStorage(vbo, vertices.size() * sizeof(Vertex), vertices.data(), 0);
			//ebo
			glCreateBuffers(1, &ebo);
			glNamedBufferStorage(ebo, indices.size() * sizeof(unsigned int), indices.data(), 0);

			//vao
			glCreateVertexArrays(1, &vao);
			glEnableVertexArrayAttrib(vao, 0);
			glEnableVertexArrayAttrib(vao, 1);
			glEnableVertexArrayAttrib(vao, 2);

			glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, false, offsetof(Vertex, position));
			glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, false, offsetof(Vertex, normal));
			glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, false, offsetof(Vertex, uv));

			glVertexArrayElementBuffer(vao, ebo);

			//Today i learned u can just do this
			glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex));

			glVertexArrayAttribBinding(vao, 0, 0);
			glVertexArrayAttribBinding(vao, 1, 0);
			glVertexArrayAttribBinding(vao, 2, 0);

			if (!static_model) {

				glEnableVertexArrayAttrib(vao, 3);
				glEnableVertexArrayAttrib(vao, 4);
				glCreateBuffers(1, &vbbo);
				glNamedBufferStorage(vbbo, vert_bones.size() * sizeof(VertexBone), vert_bones.data(), 0);

				glVertexArrayAttribIFormat(vao, 3, 4, GL_INT, offsetof(VertexBone, boneIDs));
				glVertexArrayAttribFormat(vao, 4, 4, GL_FLOAT, false, offsetof(VertexBone, weights));
				glVertexArrayVertexBuffer(vao, 1, vbbo, 0, sizeof(VertexBone));

				glVertexArrayAttribBinding(vao, 3, 1);
				glVertexArrayAttribBinding(vao, 4, 1);
			}

			glBindVertexArray(0);
			drawCnt = static_cast<GLuint>(indices.size());
			drawMode = GL_TRIANGLES;
		}


		void VertexBone::SetVertexBone(int id, float weight) {
			for (int i{}; i < MAX_BONE_INFLUENCE; ++i) {
				if (boneIDs[i] < 0) {
					boneIDs[i] = id;
					weights[i] = weight;
					return;
				}
			}
		}
	}
}