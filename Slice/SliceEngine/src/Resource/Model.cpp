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
	constexpr uint16_t version_number = 2;	//i think having a vers number could be useful, maybe
	constexpr uint64_t i_size = sizeof(unsigned int);
	constexpr uint64_t f_size = sizeof(float);
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

		/*
		void Model::combine_setup_meshes(std::vector<Mesh> const& meshes) {
			//the absolute simplest way, is to simply add up all the vertices and indices together
			//most likely do this for now since this is just a temporary soln anyways
			//premultiply the relative transform as well for each mesh - collapsing the hierachy
			//in this case the transform has already pre-multiplied the pos and norm values
			std::vector<Vertex> final_vertices{};
			std::vector<unsigned int> final_indices{};
			//get total vtx and idx cnt
			int vtx_cnt{}, idx_cnt{};
			for (auto const& m : meshes) {
				vtx_cnt += static_cast<int>(m.vertices.size());
				idx_cnt += static_cast<int>(m.indices.size());
			}
			final_vertices.reserve(vtx_cnt);
			final_indices.reserve(idx_cnt);
			int idx_offset = 0;	//offset to shift the indices values since we are combining the vertices all into 1 buffer
			for (auto const& m : meshes) {
				for (auto const& i : m.indices) {
					final_indices.emplace_back(i + idx_offset);
				}
				final_vertices.insert(final_vertices.end(), m.vertices.begin(), m.vertices.end());
				idx_offset = static_cast<int>(final_vertices.size());
			}

			//finally, setup the vbo, vao, ebo
			drawCnt = static_cast<int>(final_indices.size());
			drawMode = GL_TRIANGLES;

			//vbo
			glCreateBuffers(1, &vbo);
			glNamedBufferStorage(vbo, final_vertices.size() * sizeof(Vertex), final_vertices.data(), 0);

			//ebo
			glCreateBuffers(1, &ebo);
			glNamedBufferStorage(ebo, final_indices.size() * sizeof(unsigned int), final_indices.data(), 0);

			//vao
			glCreateVertexArrays(1, &vao);
			glEnableVertexArrayAttrib(vao, 0);
			glEnableVertexArrayAttrib(vao, 1);
			glEnableVertexArrayAttrib(vao, 2);
			glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, false, offsetof(Vertex, position));
			glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, false, offsetof(Vertex, normal));
			glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, false, offsetof(Vertex, uv));

			glVertexArrayElementBuffer(vao, ebo);

			//i learned recently
			glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex));


			glVertexArrayAttribBinding(vao, 0, 0);
			glVertexArrayAttribBinding(vao, 1, 0);
			glVertexArrayAttribBinding(vao, 2, 0);

			glBindVertexArray(0);
		}
		*/
		
		void Model::unpack_data(char* const buffer, uint64_t& offset) {
			uint32_t dest{};
			//name
			memcpy(&dest, buffer + offset, i_size); offset += i_size;
			name.resize(dest);
			memcpy(name.data(), buffer + offset, dest); offset += dest;
			//meshes
			memcpy(&dest, buffer + offset, i_size); offset += i_size;
			meshes.resize(dest);
			for (auto& m : meshes) {
				m.unpack_data(buffer, offset);
				m.setup_mesh();
			}

			//node hierachy
			rootNode.unpack_data(buffer, offset);
		}
		void Mesh::unpack_data(char* const buffer, uint64_t& offset) {
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
			return;
		}

		void Model::LoadDefaultQuadModel()
		{
			meshes.resize(1);
			auto& mesh = meshes[0];
			auto& vertices = mesh.vertices;	vertices.clear();
			vertices.reserve(4);
			vertices.emplace_back(Vertex{{-0.5, -0.5, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0}});
			vertices.emplace_back(Vertex{{ 0.5, -0.5, 0.0}, {0.0, 0.0, 1.0}, {1.0, 0.0}});
			vertices.emplace_back(Vertex{{-0.5,  0.5, 0.0}, {0.0, 0.0, 1.0}, {0.0, 1.0}});
			vertices.emplace_back(Vertex{{ 0.5,  0.5, 0.0}, {0.0, 0.0, 1.0}, {1.0, 1.0}});

			auto& indices = mesh.indices;	indices.clear();
			indices.reserve(6);
			indices.emplace_back(0); indices.emplace_back(1); indices.emplace_back(2);
			indices.emplace_back(2); indices.emplace_back(1); indices.emplace_back(3);

			mesh.setup_mesh();

			//rootNode.local_transform = glm::identity<glm::mat4>();
			rootNode.mesh_ref.resize(1);
			rootNode.mesh_ref[0] = 0;
			rootNode.children.clear();
			return;
		}

		void Model::LoadDefaultLineModel()
		{
			meshes.resize(1);
			auto& m = meshes[0];
			m.drawMode = GL_LINES;
			m.drawCnt = 2;
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
			return;
		}

		void Model::LoadDefaultFrustrumModel()
		{
			meshes.resize(1);
			auto& m = meshes[0];
			m.drawMode = GL_LINE_LOOP;
			m.drawCnt = 16;
			vtx.resize(m.drawCnt);

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
			
			return;
		}

		void Model::DestroyModel() {
			for (auto& m : meshes) {
				if (m.ebo != 0) {
					glDeleteBuffers(1, &m.ebo);
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

			drawCnt = indices.size();
			drawMode = GL_TRIANGLES;
		}
	}
}