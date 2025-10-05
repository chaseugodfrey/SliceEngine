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