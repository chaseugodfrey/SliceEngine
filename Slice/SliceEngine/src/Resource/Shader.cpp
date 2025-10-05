#include <pch.h>
#include "Shader.h"
#include <fstream>
#include <GL/glew.h>
#include <sstream>

namespace SliceEngine
{
	namespace SliceEngineTypes {
		//yoinked this code from old resource manager
		unsigned int Shader::CompileShader(const std::string& vertFile, const std::string& fragFile) {
			std::ifstream vertShaderFile(vertFile, std::ios::binary);


			if (!vertShaderFile)
			{
				SLICE_LOG_WARNING("Unable to open Vertex Shader File");
				return {};
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
				return {};
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
				return {};
			}
			GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragShader, 1, frag_shader_code, nullptr);
			glCompileShader(fragShader);
			glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(fragShader, 512, nullptr, infoLog);
				SLICE_LOG_WARNING("Unable to compile fragment shader:" + std::string(infoLog));

				return {};
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

				return {};
			}
			glValidateProgram(shader);
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 512, nullptr, infoLog);
				SLICE_LOG_WARNING("Validate Failed:" + std::string(infoLog));

				return {};
			}
			glDeleteShader(vertShader);
			glDeleteShader(fragShader);
			return shader;
		}
	
		Shader Shader::LoadShader(std::string const& filepath) {
			//std::ifstream shaderSource(filepath, std::ios::binary);


			//if (!shaderSource)
			//{
			//	SLICE_LOG_WARNING("Unable to open Shader Resource File");
			//	return {};
			//}

			//std::stringstream ss{};
			//ss << shaderSource.rdbuf();
			//std::string vert{}, frag{};

			//ss >> vert >> frag;
			//if (vert.empty() || frag.empty()) {
			//	SLICE_LOG_WARNING("Unable to read Shader Resource File");
			//	return {};
			//}

			std::filesystem::path directory(filepath);
			std::string fileName = directory.stem().string();
			std::filesystem::path parentPath = directory.parent_path();
			std::filesystem::path vertPath = parentPath / (fileName + ".vert");
			std::filesystem::path fragPath = parentPath / (fileName + ".frag");


			return { CompileShader(vertPath.string(), fragPath.string())};
		}

		void Shader::DestroyShader() {
			glDeleteProgram(s);
		}
	}
}