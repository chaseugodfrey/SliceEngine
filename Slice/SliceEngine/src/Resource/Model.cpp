#include <pch.h>
#include "Model.h"
#include <fstream>
#include <glm/gtc/type_ptr.hpp>

namespace {
	//some consts to help typing
	constexpr uint16_t version_number = 1;	//i think having a vers number could be useful, maybe
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
				vtx_cnt += m.vertices.size();
				idx_cnt += m.indices.size();
			}
			final_vertices.reserve(vtx_cnt);
			final_indices.reserve(idx_cnt);
			int idx_offset = 0;	//offset to shift the indices values since we are combining the vertices all into 1 buffer
			for (auto const& m : meshes) {
				for (auto const& i : m.indices) {
					final_indices.emplace_back(i + idx_offset);
				}
				final_vertices.insert(final_vertices.end(), m.vertices.begin(), m.vertices.end());
				idx_offset = final_vertices.size();
			}

			//finally, setup the vbo, vao, ebo
			drawCnt = final_indices.size();
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

		void Model::unpack_data(char* const buffer, uint64_t& offset) {
			uint32_t dest{};
			//name
			memcpy(&dest, buffer + offset, i_size); offset += i_size;
			name.resize(dest);
			memcpy(name.data(), buffer + offset, dest); offset += dest;
			//meshes
			memcpy(&dest, buffer + offset, i_size); offset += i_size;
			std::vector<Mesh> meshes;
			meshes.resize(dest);
			for (auto& m : meshes) {
				m.unpack_data(buffer, offset);
			}

			//Combine meshes into 1 vao, vbo, and ebo, because for now we dont split the objs into multiple meshes
			combine_setup_meshes(meshes);
		}
		void Mesh::unpack_data(char* const buffer, uint64_t& offset) {
			uint32_t dest{};

			//name
			memcpy(&dest, buffer + offset, i_size); offset += i_size;
			name.resize(dest);
			memcpy(name.data(), buffer + offset, dest); offset += dest;

			uint32_t vert_size{}, idx_size{}, tex_size{};
			uint32_t num_vert{}, num_idx{}, num_tex{};

			//buffer details
			memcpy(&vert_size, buffer + offset, i_size); offset += i_size;
			memcpy(&num_vert, buffer + offset, i_size); offset += i_size;

			memcpy(&idx_size, buffer + offset, i_size); offset += i_size;
			memcpy(&num_idx, buffer + offset, i_size); offset += i_size;

			memcpy(&tex_size, buffer + offset, i_size); offset += i_size;
			memcpy(&num_tex, buffer + offset, i_size); offset += i_size;

			//buffer
			uint64_t vert_buffer_size = vert_size * num_vert;
			uint64_t idx_buffer_size = idx_size * num_idx;
			uint64_t tex_buffer_size = tex_size * num_tex;

			vertices.resize(num_vert);
			indices.resize(num_idx);
			textures.resize(num_tex);
			memcpy(vertices.data(), buffer + offset, vert_buffer_size); offset += vert_buffer_size;
			memcpy(indices.data(), buffer + offset, idx_buffer_size); offset += idx_buffer_size;
			memcpy(textures.data(), buffer + offset, tex_buffer_size); offset += tex_buffer_size;
			memcpy(glm::value_ptr(transform), buffer + offset, sizeof(glm::mat4x4)); offset += sizeof(glm::mat4x4);
		}

		//yoinked from old resource manager
		bool Model::LoadModel(std::string const& filePath) {
			std::ifstream ifs(filePath, std::ios::binary);

			if (!ifs)
			{
				SLICE_LOG_WARNING("Unable to open Obj:" + filePath);
				return false;
			}

			// GL_TRIANGLES, GL_TRIANGLE_FAN, GL_TRIANGLE_STRIP
			ifs >> drawMode >> drawCnt;
			if (drawMode == 0)
			{
				SLICE_LOG_WARNING("Error reading obj file");
				ifs.close();
				return false;
			}

			switch (drawMode)
			{
			case GL_LINES:
				__fallthrough;
			case GL_LINE_LOOP:
			{
				vtx.reserve(drawCnt);
				float v1, v2, v3;
				for (unsigned int i{}; i < drawCnt; ++i)
				{
					ifs >> v1 >> v2 >> v3;
					vtx.emplace_back(glm::vec3{ v1, v2, v3 });
				}
				ifs.close();

				glCreateBuffers(1, &vbo);
				glNamedBufferStorage(vbo, vtx.size() * sizeof(glm::vec3), vtx.data(), GL_DYNAMIC_STORAGE_BIT);
				glCreateVertexArrays(1, &vao);
				// layout=0
				glEnableVertexArrayAttrib(vao, 0);
				glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(glm::vec3));
				glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
				glVertexArrayAttribBinding(vao, 0, 0);
				break;
			}
			case GL_TRIANGLES:
				__fallthrough;
			case GL_TRIANGLE_STRIP:
			{
				std::vector<float> tmpVtx;
				tmpVtx.reserve(drawCnt * 8);
				float v1;
				for (unsigned int i{}; i < drawCnt * 8; ++i)
				{
					ifs >> v1;
					tmpVtx.emplace_back(v1);
				}
				ifs.close();
				glCreateBuffers(1, &vbo);
				glNamedBufferStorage(vbo, tmpVtx.size() * sizeof(float), tmpVtx.data(), GL_DYNAMIC_STORAGE_BIT);
				glCreateVertexArrays(1, &vao);
				// layout=0
				glEnableVertexArrayAttrib(vao, 0);
				glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(float) * 8);
				glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
				glVertexArrayAttribBinding(vao, 0, 0);
				// layout=1 Normal
				glEnableVertexArrayAttrib(vao, 1);
				glVertexArrayVertexBuffer(vao, 1, vbo, sizeof(float) * 3, sizeof(float) * 8);
				glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, 0);
				glVertexArrayAttribBinding(vao, 1, 1);
				// layout=2 Texture
				glEnableVertexArrayAttrib(vao, 2);
				glVertexArrayVertexBuffer(vao, 2, vbo, sizeof(float) * 6, sizeof(float) * 8);
				glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, GL_FALSE, 0);
				glVertexArrayAttribBinding(vao, 2, 2);
				break;
			}
			}

			return true;
		}

		void Model::DestroyModel() {
			glDeleteBuffers(1, &vbo);
			glDeleteVertexArrays(1, &vao);
		}
	}
}