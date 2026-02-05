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
		// What I Should See Inside .cshader File
		// ---------- e.g. 1 ---------- Default
		// # COLOR CODE
		// Mul_Vec4: TexCol, Col / Var_0
		// END: Var_0
		// 
		// # ROUGHNESS CODE
		// Vec2_f_f: roughness, metallic / VarR_0
		// END: VarR_0
		// ---------- e.g. 2 ---------- PillarShader
		// # COLOR CODE
		// fRand_Vec2: vTex / Var_0
		// Mul_f: Var_0, noiseScale / Var_1
		// 
		// GetY_Vec2: vTex / Var_2
		// SmoothStep_f: 0.f, Var_2, Var_1 / Var_3
		// 
		// GetA_Vec4: texCol / Var_4
		// GetA_Vec4: color / Var_5
		// Mul_f: Var_3, Var_4 / Var_6
		// Mul_f: Var_6, Var_5 / Var_7
		// 
		// Mul_Vec4: texCol, color / Var_8
		// SetA_Vec4: Var_8, 0.f
		// 
		// GetX_Vec2: vTex / Var_9
		// Vec4_f_f_f_f: Var_9, Var_9, Var_9, Var_9 / Var_10
		// Mul_Vec4: Var_8, Var_10 / Var_11
		// 
		// SetA_Vec4: Var_11, Var_7
		// END: Var_11
		// 
		// # ROUGHNESS CODE
		// END: vec2(0.f)
		// 
		//-------------------------------------------------------------------------
		// Okay, so, how do I tell what is the order of these functions w/ ids
		//-------------------------------------------------------------------------
		//
		// Step 1: Gather the lines & store the IDs
		// Step 2: Map ids -> What ID I Free + (cShaderFuncsTemplates + dataIDS), ++ Check my Dependencies
		// 
		// Step 3: Extract funcs with 0 Dependencies, and loop through all functions w/ it Dependencies?
		// 
		// e.g.
		//  -- CShaderFunc + sprintf --		  -- Ids --			-- Counter --
		// 1 : texCol, color				Frees[2,4]		Dependencies Remaining(0)
		// 2 : 1, 4							Frees[3,5]		Dependencies Remaining(2)
		// 3 : 2, 4							Frees[5]		Dependencies Remaining(2)
		// 4 : 1, color						Frees[1]		Dependencies Remaining(1)
		// 5 : 2, 3							Frees[]			Dependencies Remaining(2)
		// 
		std::unordered_map<std::string, std::string> cShaderPredefines
		{
			{"frand_Vec2", "float frand_vec2(vec2 n) {return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);}"},
			{"sat_f", "float sat_f(float x) {return clamp(x, 0.0, 1.0);}"},
			{"sat_Vec3", "vec3 sat_Vec3(vec3 x) {return clamp(x, vec3(0.0), vec3(1.0));}"},
			{"SetV4F", R"(vec4 SetV4F(vec4 inv, float val, int n){
switch(n){
	case 0: return vec4(val, inv.gba);
	case 1: return vec4(inv.r, val, inv.ba);
	case 2: return vec4(inv.rg, val, inv.a);
	case 3: return vec4(inv.rgb, val);
}})"}
		};

		std::unordered_map<std::string, cShaderFunc> cShaderFuncsTemplates{
			{"END_COLOR", {"finalCol = %s;\n", "", ShaderGraphFunc_T::IMMUTABLE, CSHAD_T::NIL, {CSHAD_T::VEC4}}},
			{"END_MET_ROUGH", {"roughMet = %s;\n", "", ShaderGraphFunc_T::IMMUTABLE, CSHAD_T::NIL, {CSHAD_T::VEC2}}},

			{"Vec2_f_f", {"vec2 %s = vec2(%s, %s);\n", "", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::VEC2, {CSHAD_T::FLOAT, CSHAD_T::FLOAT}}},
			{"Vec3_f_f_f", {"vec3 %s = vec3(%s, %s, %s);\n", "", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::VEC3, {CSHAD_T::FLOAT, CSHAD_T::FLOAT, CSHAD_T::FLOAT}}},
			{"Vec4_f_f_f_f", {"vec4 %s = vec4(%s, %s, %s, %s);\n", "", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::VEC4, {CSHAD_T::FLOAT, CSHAD_T::FLOAT, CSHAD_T::FLOAT, CSHAD_T::FLOAT}}},
			
			{"GetX_Vec2", {"float %s = %s.x;\n", "", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::FLOAT, {CSHAD_T::VEC2}}},
			{"GetY_Vec2", {"float %s = %s.y;\n", "", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::FLOAT, {CSHAD_T::VEC2}}},
			{"GetX_Vec3", {"float %s = %s.x;\n", "", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::FLOAT, {CSHAD_T::VEC3}}},
			{"GetY_Vec3", {"float %s = %s.y;\n", "", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::FLOAT, {CSHAD_T::VEC3}}},
			{"GetZ_Vec3", {"float %s = %s.z;\n", "", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::FLOAT, {CSHAD_T::VEC3}}},
			{"GetX_Vec4", {"float %s = %s.x;\n", "", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::FLOAT, {CSHAD_T::VEC4}}},
			{"GetY_Vec4", {"float %s = %s.y;\n", "", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::FLOAT, {CSHAD_T::VEC4}}},
			{"GetZ_Vec4", {"float %s = %s.z;\n", "", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::FLOAT, {CSHAD_T::VEC4}}},
			{"GetA_Vec4", {"float %s = %s.a;\n", "", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::FLOAT, {CSHAD_T::VEC4}}},

			{"SetR_Vec4", {"vec4 %s = SetV4F(%s, %s, 0);\n", "SetV4F", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::VEC4, {CSHAD_T::VEC4, CSHAD_T::FLOAT}}},
			{"SetG_Vec4", {"vec4 %s = SetV4F(%s, %s, 1);\n", "SetV4F", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::VEC4, {CSHAD_T::VEC4, CSHAD_T::FLOAT}}},
			{"SetB_Vec4", {"vec4 %s = SetV4F(%s, %s, 2);\n", "SetV4F", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::VEC4, {CSHAD_T::VEC4, CSHAD_T::FLOAT}}},
			{"SetA_Vec4", {"vec4 %s = SetV4F(%s, %s, 3);\n", "SetV4F", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::VEC4, {CSHAD_T::VEC4, CSHAD_T::FLOAT}}},

			{"sat_f", {"float %s = sat_f(%s);\n", "sat_f", ShaderGraphFunc_T::MATH, CSHAD_T::FLOAT,{CSHAD_T::FLOAT}}},
			{"sat_Vec3", {"vec3 %s = sat_Vec3(%s);\n", "sat_Vec3", ShaderGraphFunc_T::MATH, CSHAD_T::VEC3,{CSHAD_T::VEC3}}},

			{"Mul_f", {"float %s = %s * %s;\n", "", ShaderGraphFunc_T::MATH, CSHAD_T::FLOAT, {CSHAD_T::FLOAT, CSHAD_T::FLOAT}}},
			{"Mul_Vec2", {"vec2 %s = %s * %s;\n", "", ShaderGraphFunc_T::MATH, CSHAD_T::VEC2, {CSHAD_T::VEC2, CSHAD_T::VEC2}}},
			{"Mul_Vec3", {"vec3 %s = %s * %s;\n", "", ShaderGraphFunc_T::MATH, CSHAD_T::VEC3, {CSHAD_T::VEC3, CSHAD_T::VEC3}}},
			{"Mul_Vec4", {"vec4 %s = %s * %s;\n", "", ShaderGraphFunc_T::MATH, CSHAD_T::VEC4, {CSHAD_T::VEC4, CSHAD_T::VEC4}}},
			
			{"SmoothStep_f", {"float %s = smoothstep(%s, %s, %s);\n", "", ShaderGraphFunc_T::MATH, CSHAD_T::FLOAT, {CSHAD_T::FLOAT,CSHAD_T::FLOAT,CSHAD_T::FLOAT}}},
			
			{"fRand_Vec2", {"float %s = frand_vec2(%s);\n", "frand_Vec2", ShaderGraphFunc_T::UTILITIES, CSHAD_T::FLOAT, {CSHAD_T::VEC2}}}
		};
		// ----- Inside LoadCShader Func =====
		std::unordered_map<std::string, CSHAD_T> dataIDS
		{
			{"vPos", CSHAD_T::VEC3},
			{"vNom", CSHAD_T::VEC3},
			{"vTex", CSHAD_T::VEC2},
			{"texCol", CSHAD_T::VEC4},
			{"color", CSHAD_T::VEC4},
			// -- Defines? --
			{"0.f", CSHAD_T::FLOAT},
			{"vec2(0.f)", CSHAD_T::VEC2}
		};
		// -----------------------------------------------------------------
		CustomShader CustomShader::LoadCShader(std::string const& filepath)
		{
			std::ifstream fragShaderFile(filepath, std::ios::binary);
			if (!fragShaderFile)
			{
				SLICE_LOG_WARNING("Unable to open Fragment Shader File");
				return {};
			}
			std::vector<ShaderParams> dataIn;
			nlohmann::json cshaderJson;
			try
			{
				cshaderJson = nlohmann::json::parse(fragShaderFile);
			}
			catch (nlohmann::json::parse_error& e)
			{
				fragShaderFile.close();
				SLICE_LOG_ERROR("Invalid cshader JSON file" + std::string(e.what()));
				return {};
			}
			fragShaderFile.close();
			
			// 1. Extract Params
			nlohmann::json paramsJson = cshaderJson["Params"];
			std::unordered_map<std::string, CSHAD_T> dataI = dataIDS; // copies :p

			if (paramsJson.contains("Floats"))
				for (auto& [name, components] : paramsJson["Floats"].items())
				{
					ShaderParams inParam{};
					inParam.dataType = SP_TYPE::FLOAT;
					inParam.name = name;
					inParam.baseData = components.get<float>();
					dataI[name] = CSHAD_T::FLOAT;
					dataIn.push_back(inParam);
				}
			if (paramsJson.contains("Ints"))
				for (auto& [name, components] : paramsJson["Ints"].items())
				{
					ShaderParams inParam{};
					inParam.dataType = SP_TYPE::INT;
					inParam.name = name;
					inParam.baseData = components.get<int32_t>();
					dataI[name] = CSHAD_T::INT;
					dataIn.push_back(inParam);
				}
			if (paramsJson.contains("Uints"))
				for (auto& [name, components] : paramsJson["Uints"].items())
				{
					ShaderParams inParam{};
					inParam.dataType = SP_TYPE::UINT;
					inParam.name = name;
					inParam.baseData = components.get<uint32_t>();
					dataI[name] = CSHAD_T::UINT;
					dataIn.push_back(inParam);
				}
			if (paramsJson.contains("Bools"))
				for (auto& [name, components] : paramsJson["Bools"].items())
				{
					ShaderParams inParam{};
					inParam.dataType = SP_TYPE::BOOL;
					inParam.name = name;
					inParam.baseData = components.get<bool>();
					dataI[name] = CSHAD_T::BOOL;
					dataIn.push_back(inParam);
				}

			// Extract Functions
			std::unordered_map<std::string, std::string> fragInclFunctions{};
			std::string fragMainShaderSource{
R"(void CustomCalc(in vec4 texCol, in vec4 color, inout vec4 finalCol, inout vec2 roughMet)
{
)"};
			LoadCShaderFunctions(fragMainShaderSource, fragInclFunctions, dataI, cshaderJson["Main"]);

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

uniform int translucentIDOnly;
uniform float translucentSelectThreshold;

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
			// Handling the SSBO elements
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
			{
				std::stringstream ss;
				for (int i = 0; i < dataIn.size(); ++i)
				{
					switch (dataIn[i].dataType)
					{
					case SliceEngineTypes::CustomShader::SP_TYPE::BOOL:
						ss << "bool " << dataIn[i].name << " = bool(ExtractUint(" << i << "));\n";
						break;
					case SliceEngineTypes::CustomShader::SP_TYPE::UINT:
						ss << "uint " << dataIn[i].name << " = ExtractUint(" << i << ");\n";
						break;
					case SliceEngineTypes::CustomShader::SP_TYPE::INT:
						ss << "int " << dataIn[i].name << " = int(ExtractUint(" << i << "));\n";
						break;
					case SliceEngineTypes::CustomShader::SP_TYPE::FLOAT:
						ss << "float " << dataIn[i].name << " = ExtractFloat(" << i << "); \n";
						break;
					}
				}
				fragNumExtraElems += ss.str();
			}
			// GLSL Main Func
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

	vec4 texColor = texture(textures[iDat[vInstance].textureID], vTex);
	if(texColor.a == 0.f)
		discard;

	vec4 finalCol = vec4(0.f);
	vec2 roughMetal = vec2(0.f);
	CustomCalc(texColor, color, fFragColor, roughMetal);

	if(translucentIDOnly == 1 && fFragColor.a < translucentSelectThreshold)
		discard;

	fGID = iDat[vInstance].entityID;
	fMetalRoughData.xy = roughMetal;
})"};

			// Combine all the texts
			fragStart += fragNumExtraElems;
			for (auto& i : fragInclFunctions)
				fragStart += cShaderPredefines.find(i.first)->second + "\n";
			fragStart += fragMainShaderSource + fragEnd;
			GLchar const* frag_shader_code[] = { fragStart.c_str() };

			// -----------------------------------------------------------
			// Start GLSL compiling
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

		namespace
		{
			struct CShadDependencies
			{
				std::vector<std::string> freesList;
				std::string funcStr{};
				uint8_t dependenciesRemaining{};
			};
		}

		void CustomShader::LoadCShaderFunctions(std::string& ret, std::unordered_map<std::string, std::string>& funcsPre, const std::unordered_map<std::string, CSHAD_T>& defaulParmas, nlohmann::json& in)
		{
			std::map<std::string, CShadDependencies> dependenciesLockedLines;
			std::queue<std::string> toClearLines;

			for (auto& [funcName, components] : in.items())
			{
				auto funcDetails = cShaderFuncsTemplates.find(funcName)->second;
				// Predefines check
				if (funcDetails.opPredefine != "" &&
					cShaderPredefines.find(funcDetails.opPredefine) != cShaderPredefines.end())
					funcsPre[funcDetails.opPredefine] = cShaderPredefines.find(funcDetails.opPredefine)->second;
				// Loop through
				for (auto& [id, dependicies] : components.items())
				{
					// id - the return id of that line
					std::vector<std::string> dep;
					dependicies.get_to(dep);

					CShadDependencies newID{};
					if (dependenciesLockedLines.find(id) != dependenciesLockedLines.end())
					{
						newID = dependenciesLockedLines[id];
					}
					newID.funcStr = funcDetails.code;

					if (funcDetails.outType != CSHAD_T::NIL)
					{
						auto p = newID.funcStr.find("%s");
						if (p != std::string::npos)
						{
							newID.funcStr.replace(p, 2, id);
						}
					}
					for (auto i : dep)
					{
						if (defaulParmas.find(i) == defaulParmas.end())
						{
							if (dependenciesLockedLines.find(i) != dependenciesLockedLines.end())
								dependenciesLockedLines[i].freesList.push_back(id);
							else
							{
								CShadDependencies othID{};
								othID.freesList.push_back(id);
								dependenciesLockedLines[i] = othID;
							}
							++newID.dependenciesRemaining;
						}
						
						auto p = newID.funcStr.find("%s");
						if (p != std::string::npos)
						{
							newID.funcStr.replace(p, 2, i);
						}
					}
					dependenciesLockedLines[id] = newID;
					if (newID.dependenciesRemaining == 0)
						toClearLines.push(id);
				}
			}
			// Extract Functions
			while (!toClearLines.empty())
			{
				auto i = toClearLines.front();
				toClearLines.pop();

				for (auto cl : dependenciesLockedLines[i].freesList)
				{
					auto fl = dependenciesLockedLines.find(cl);
					if (fl != dependenciesLockedLines.end())
					{
						if (--fl->second.dependenciesRemaining == 0)
						{
							toClearLines.push(fl->first);
						}
					}
				}
				ret += dependenciesLockedLines[i].funcStr;
			}
			ret += "}\n";
			//SLICE_LOG("\nCShaderCode: [" + ret + "]\n");
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