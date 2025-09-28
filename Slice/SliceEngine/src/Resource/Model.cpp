#include <pch.h>
#include "Model.h"
#include <fstream>

namespace SliceEngine
{
	namespace SliceEngineTypes
	{
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

			vtx.reserve(drawCnt);
			float v1, v2, v3;
			for (unsigned int i{}; i < drawCnt; ++i)
			{
				ifs >> v1 >> v2 >> v3;
				vtx.emplace_back(glm::vec3{ v1, v2, v3 });
			}
			ifs.close();

			glCreateBuffers(1, &vbo);
			glNamedBufferStorage(vbo, vtx.size() * sizeof(glm::vec3), vtx.data(), GL_MAP_WRITE_BIT);
			glCreateVertexArrays(1, &vao);
			// layout=0
			glEnableVertexArrayAttrib(vao, 0);
			glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(glm::vec3));
			glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
			glVertexArrayAttribBinding(vao, 0, 0);

			return true;
		}

		void Model::DestroyModel() {
			glDeleteBuffers(1, &vbo);
			glDeleteVertexArrays(1, &vao);
		}
	}
}