/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Shader.cpp
 author:		Elton leosantosa
 email:			leosantosa@digipen.edu
 brief:			Loads Shaders

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "Shader.h"
#include <fstream>
#include <GL/glew.h>
#include <sstream>
#include "Core/Core.h"

namespace SliceEngine
{
	namespace SliceEngineTypes {
#pragma region Shader Actual
		unsigned int Shader::CompileShader(unsigned int vertShader, unsigned int fragShader, unsigned int geomShader)
		{
			GLuint shader = glCreateProgram();
			glAttachShader(shader, vertShader);
			glAttachShader(shader, fragShader);
			if(geomShader != 0)
				glAttachShader(shader, geomShader);
			glLinkProgram(shader);

			int success;
			char infoLog[512];
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

			return shader;
		}
		Shader Shader::LoadShader(std::string const& filepath) {
			std::ifstream inFile(filepath);

			try
			{
				nlohmann::json metaData;
				inFile >> metaData;
				std::string VertexShaderName = metaData["VertexShader"].get<std::string>();
				std::string FragmentShaderName = metaData["FragmentShader"].get<std::string>();
				std::string GeometryShaderName = metaData["GeometryShader"].get<std::string>();
				unsigned int v = Core::GetInstance()->GetResourceManager()->get<VertShader>(VertexShaderName).get()->s;
				unsigned int f = Core::GetInstance()->GetResourceManager()->get<FragShader>(FragmentShaderName).get()->s;
				unsigned int g{};
				if(GeometryShaderName != "")
					g = Core::GetInstance()->GetResourceManager()->get<GeomShader>(GeometryShaderName).get()->s;
				return { CompileShader(v, f, g) };

			}
			catch (nlohmann::json::parse_error& e)
			{
				const char* errorMessageCStr = e.what();
				SLICE_LOG_ERROR("Failed to parse .meta file: {}" + std::string(errorMessageCStr));
			}
			inFile.close();

			return { };
		}

		void Shader::DestroyShader() {
			glDeleteProgram(s);
		}
#pragma endregion
#pragma region VertShader
		VertShader VertShader::LoadVertShader(std::string const& filepath)
		{
			std::ifstream vertShaderFile(filepath, std::ios::binary);

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

			return { vertShader };
		}
		void VertShader::DestroyVertShader()
		{
			glDeleteShader(s);
		}
#pragma endregion
#pragma region GeomShader
		GeomShader GeomShader::LoadGeomShader(std::string const& filepath)
		{
			std::ifstream geomShaderFile(filepath, std::ios::binary);
			if (!geomShaderFile)
			{
				SLICE_LOG_WARNING("Unable to open Geometry Shader File");
				return {};
			}
			std::string geomShaderSource;
			geomShaderFile.seekg(0, std::ios::end);
			geomShaderSource.resize(geomShaderFile.tellg());
			geomShaderFile.seekg(0, std::ios::beg);
			geomShaderFile.read(&geomShaderSource[0], geomShaderSource.size());
			geomShaderFile.close();
			GLchar const* geom_shader_code[] = { geomShaderSource.c_str() };

			int success;
			char infoLog[512];
			GLuint geomShader = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geomShader, 1, geom_shader_code, nullptr);
			glCompileShader(geomShader);
			glGetShaderiv(geomShader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(geomShader, 512, nullptr, infoLog);
				SLICE_LOG_WARNING("Unable to compile geometry shader:" + std::string(infoLog));

				return {};
			}
			return { geomShader };
		}
		void GeomShader::DestroyGeomShader()
		{
			glDeleteShader(s);
		}
#pragma endregion
#pragma region FragShader
		FragShader FragShader::LoadFragShader(std::string const& filepath)
		{
			std::ifstream fragShaderFile(filepath, std::ios::binary);
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

			int success;
			char infoLog[512];
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
			return { fragShader };
		}
		void FragShader::DestroyFragShader()
		{
			glDeleteShader(s);
		}
#pragma endregion
}
}