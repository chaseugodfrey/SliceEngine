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
#pragma region CustomShader
		CustomShader CustomShader::LoadCShader(std::string const& filepath)
		{
			std::ifstream fragShaderFile(filepath, std::ios::binary);
			if (!fragShaderFile)
			{
				SLICE_LOG_WARNING("Unable to open Fragment Shader File");
				return {};
			}
			std::string fragShaderSource;
			std::string line;
			std::vector<ShaderParams> dataIn;
			bool readingIn = false;
			while (std::getline(fragShaderFile, line))
			{
				if (!line.empty() && line.back() == '\r')
					line.pop_back();
				if (line == std::string{ "#CODE" })
					break;
				if (line == std::string{ "" })
					continue;
				if (readingIn)
				{
					ShaderParams inParam{};
					std::string throwaway;
					std::stringstream ss{ line };
					ss >> throwaway; // data type
					if (throwaway[0] == 'u')
						inParam.isUnsigned = true;

					if (throwaway == "uint" || throwaway == "int")
						inParam.numBytes = 4;
					else if(throwaway == "bool")
						inParam.numBytes = 1;
					else if (throwaway == "float")
					{
						inParam.isFloating = true;
						inParam.numBytes = 4;
					}
					ss >> inParam.name;
					ss >> throwaway;

					if (inParam.isFloating)
					{
						float tempFloat{};
						ss >> tempFloat;
						inParam.baseData = std::bit_cast<uint32_t>(tempFloat);
					}
					else
						ss >> inParam.baseData;

					dataIn.push_back(inParam);
				}
				else if (line == std::string("#IN"))
					readingIn = true;
			}
			auto midPos = fragShaderFile.tellg();
			fragShaderFile.seekg(0, std::ios::end);
			auto fileSize = fragShaderFile.tellg() - midPos;
			fragShaderSource.resize(fileSize);
			fragShaderFile.seekg(midPos, std::ios::beg);
			fragShaderFile.read(&fragShaderSource[0], fragShaderSource.size());
			fragShaderFile.close();

			std::string fragStart{
R"(#version 460 core
// Custom Shader
#extension GL_ARB_bindless_texture : require

layout (location=0) in vec3 vPos; // In M Space
layout (location=1) in vec3 vNom; // In MV Space
layout (location=2) in vec2 vTex;
layout (location=3) in flat uint vInstance;

layout (location=0)	out vec4 fFragColor; // location 0 is default GL_BACK_LEFT color buffer
layout (location=1) out uint fGID;
layout (location=2) out vec3 fPositionData;
layout (location=3) out vec3 fNormalData;
layout (location=4) out vec4 fMetalRoughData;

struct BasicIDat
{
	mat4 mdlMtx;
	uint entityID;
	uint textureID; 
	uint tex2ID;
	uint col;
};

layout(binding=0, std430) readonly buffer ssbo0
{
	sampler2D textures[];
};

layout(binding=1, std430) readonly buffer ssbo1
{
	BasicIDat iDat[];
};

layout(binding=2, std430) readonly buffer ssbo2
{
	uvec4 eDat[];
};
)"};
			std::string fragNumExtraElems{};
			{
				std::stringstream ss;
				ss << "\nint numElem = " << dataIn.size() << ";\n";
				fragNumExtraElems = ss.str();
			}
			fragNumExtraElems += std::string{
R"(
uint ExtractUint(int num)
{
	uint mainID = (vInstance * numElem + num) / 4;
	uint subID = (vInstance * numElem + num) % 4;
	return eDat[mainID][subID];
}
float ExtractFloat(int num)
{
	uint mainID = (vInstance * numElem + num) / 4;
	uint subID = (vInstance * numElem + num) % 4;
	return uintBitsToFloat(eDat[mainID][subID]);
}
)"};
			std::string fragEnd{
R"(
void main(void){
	fPositionData = vPos;
	fNormalData = normalize(vNom);
	vec4 color = vec4(
 float(iDat[vInstance].col >> 24 & 0xFF),
 float(iDat[vInstance].col >> 16 & 0xFF),
 float(iDat[vInstance].col >> 8 & 0xFF),
 float(iDat[vInstance].col & 0xFF)) / float(0xFF);

	fFragColor = TexColorC(texture(textures[iDat[vInstance].textureID], vTex), color);
	fGID = iDat[vInstance].entityID;
	fMetalRoughData.xy = RoughMet();
})"};

			fragStart += fragNumExtraElems + fragShaderSource + fragEnd;
			GLchar const* frag_shader_code[] = { fragStart.c_str() };

			int success;
			char infoLog[512];
			GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragShader, 1, frag_shader_code, nullptr);
			glCompileShader(fragShader);
			glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(fragShader, 512, nullptr, infoLog);
				SLICE_LOG_WARNING("Unable to compile custom fragment shader:" + std::string(infoLog));

				return {};
			}

			//*********************************************************
			std::string VertexShaderName{"Shaders/deferred_Vert.vert"};
			//*********************************************************
			
			GLuint shader = glCreateProgram();
			glAttachShader(shader, Core::GetInstance()->GetResourceManager()->get<VertShader>(VertexShaderName).get()->s);
			glAttachShader(shader, fragShader);
			glLinkProgram(shader);

			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 512, nullptr, infoLog);
				SLICE_LOG_WARNING("Link / Compile Custom Shader Failed:" + std::string(infoLog));
				glDeleteShader(fragShader);
				return {};
			}
			glValidateProgram(shader);
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 512, nullptr, infoLog);
				SLICE_LOG_WARNING("Validate Custom Shader Failed:" + std::string(infoLog));
				glDeleteShader(fragShader);
				return {};
			}

			glDeleteShader(fragShader);
			return { shader, dataIn };
		}
		void CustomShader::DestroyCShader()
		{
			glDeleteShader(s);
			s = 0;
			dataIn.clear();
		}
#pragma endregion
}
}