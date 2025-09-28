#include <pch.h>
#include "Model.h"
#include <fstream>

namespace SliceEngine
{
	namespace SliceEngineTypes
	{
		//yoinked from old resource manager
		Model LoadModel(std::string const& filePath) {
			std::ifstream ifs(filePath, std::ios::binary);

			Model m{};
			if (!ifs)
			{
				SLICE_LOG_WARNING("Unable to open Obj:" + filePath);
				return m;
			}

			// GL_TRIANGLES, GL_TRIANGLE_FAN, GL_TRIANGLE_STRIP
			ifs >> m.drawMode >> m.drawCnt;
			if (m.drawMode == 0)
			{
				SLICE_LOG_WARNING("Error reading obj file");
				ifs.close();
				return m;
			}

			m.vtx.reserve(m.drawCnt);
			float v1, v2, v3;
			for (unsigned int i{}; i < m.drawCnt; ++i)
			{
				ifs >> v1 >> v2 >> v3;
				m.vtx.emplace_back(glm::vec3{ v1, v2, v3 });
			}
			ifs.close();

			glCreateBuffers(1, &m.vbo);
			glNamedBufferStorage(m.vbo, m.vtx.size() * sizeof(glm::vec3), m.vtx.data(), GL_MAP_WRITE_BIT);
			glCreateVertexArrays(1, &m.vao);
			// layout=0
			glEnableVertexArrayAttrib(m.vao, 0);
			glVertexArrayVertexBuffer(m.vao, 0, m.vbo, 0, sizeof(glm::vec3));
			glVertexArrayAttribFormat(m.vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
			glVertexArrayAttribBinding(m.vao, 0, 0);

			return m;
		}

		void Model::DestroyModel() {
			glDeleteBuffers(1, &vbo);
			glDeleteVertexArrays(1, &vao);
		}
	}
}