#include <pch.h>
#include "ResourceManager.h"
#include <fstream>

namespace SliceEngine
{
	//SLICE_LOG("Creating Main Window.");
	ResourceManager::ResourceManager() : mOnlyShader(), mOnlyModel(){}

	ResourceManager::~ResourceManager()
	{
		glDeleteProgram(mOnlyShader.s);

		glDeleteVertexArrays(1, &mOnlyModel.vao);
		glDeleteBuffers(1, &mOnlyModel.vbo);
	}

	void ResourceManager::LoadShader(const std::string& vertFile, const std::string& fragFile)
	{
		std::ifstream vertShaderFile(vertFile, std::ios::binary);


		if (!vertShaderFile)
		{
			SLICE_LOG_WARNING("Unable to open Vertex Shader File");
			return;
		}
		std::string vertShaderSource;
		vertShaderFile.seekg(0, std::ios::end);
		vertShaderSource.resize(vertShaderFile.tellg());
		vertShaderFile.seekg(0, std::ios::beg);
		vertShaderFile.read(&vertShaderSource[0], vertShaderSource.size());
		vertShaderFile.close();
		GLchar const* vert_shader_code[] = { vertShaderSource.c_str() };

		std::ifstream fragShaderFile(fragFile, std::ios::binary);
		if (!fragShaderFile)
		{
			SLICE_LOG_WARNING("Unable to open Fragment Shader File");
			return;
		}
		std::string fragShaderSource;
		fragShaderFile.seekg(0, std::ios::end);
		fragShaderSource.resize(fragShaderFile.tellg());
		fragShaderFile.seekg(0, std::ios::beg);
		fragShaderFile.read(&fragShaderSource[0], fragShaderSource.size());
		fragShaderFile.close();
		GLchar const* frag_shader_code[] = { fragShaderSource.c_str() };


		GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertShader, 1, vert_shader_code, nullptr);
		glCompileShader(vertShader);

		int success;
		char infoLog[512];
		glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertShader, 512, nullptr, infoLog);
			SLICE_LOG_WARNING("Unable to compile vertex shader:" + std::string(infoLog));
			return;
		}
		GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragShader, 1, frag_shader_code, nullptr);
		glCompileShader(fragShader);
		glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragShader, 512, nullptr, infoLog);
			SLICE_LOG_WARNING("Unable to compile fragment shader:" + std::string(infoLog));
			return;
		}
		GLuint shader = glCreateProgram();
		glAttachShader(shader, vertShader);
		glAttachShader(shader, fragShader);
		glLinkProgram(shader);

		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 512, nullptr, infoLog);
			SLICE_LOG_WARNING("Link / Compile Failed:" + std::string(infoLog));
			return;
		}
		glValidateProgram(shader);
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 512, nullptr, infoLog);
			SLICE_LOG_WARNING("Validate Failed:" + std::string(infoLog));
			return;
		}
		glDeleteShader(vertShader);
		glDeleteShader(fragShader);
		mOnlyShader.s = shader;
	}

	void ResourceManager::LoadModel(const std::string& file)
	{
		std::ifstream ifs(file, std::ios::binary);
		if (!ifs)
		{
			SLICE_LOG_WARNING("Unable to open Obj:" + file);
			return;
		}
		Model m;

		// GL_TRIANGLES, GL_TRIANGLE_FAN, GL_TRIANGLE_STRIP
		ifs >> m.drawMode >> m.drawCnt;
		if (m.drawMode == 0)
		{
			SLICE_LOG_WARNING("Error reading obj file");
			ifs.close();
			return;
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

		mOnlyModel = m;
	}

	Shader& ResourceManager::GetShader()
	{
		return mOnlyShader;
	}

	Model& ResourceManager::GetModel()
	{
		return mOnlyModel;
	}

}