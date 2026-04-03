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
		static std::unordered_map<std::string, std::string> cShaderPredefines
		{
			{"flipY", "vec2 flipY(vec2 n) {return vec2(n.x, 1.f-n.y);}"},
			{"frand_Vec2", "float frand_vec2(vec2 n) {return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);}"},
			{"sat_Vec4", "vec3 sat_Vec4(vec4 x) {return clamp(x, vec4(0.0), vec4(1.0));}"},
			{"extractNom", R"(vec3 extractNom(vec2 v){
	float z = sqrt(1.0 - clamp(v.x * v.x + v.y * v.y, 0.0, 1.0));
    vec3 actlNom = normalize(vec3(v.x, v.y, z));
	return normalize(TBN * (actlNom * 2.0f - 1.0f));
})"},
			{"OffsetTexture", R"(vec4 OffsetTexture(sampler2D tex, float uStart, float uEnd, float vStart, float vEnd) {
	vec2 mapUV = mix(vec2(uStart, vStart), vec2(uEnd, vEnd), vUV);
	return texture(tex, mapUV);
})"},
			{"SetV4F", R"(vec4 SetV4F(vec4 inv, float val, int n){
switch(n){
	case 0: return vec4(val, inv.gba);
	case 1: return vec4(inv.r, val, inv.ba);
	case 2: return vec4(inv.rg, val, inv.a);
	case 3: return vec4(inv.rgb, val);
}})"},
			{"GradientNoise_Deterministic", R"(
uint Hash_Tchou_2_1_uint(uvec2 v)
{
	v.y ^= 1103515245U;
	v.x += v.y;
	v.x *= v.y;
	v.x ^= v.x >> 5u;
	v.x *= 0x27d4eb2du;
	return v.x;
}

float Hash_Tchou_2_1_float(vec2 i)
{
	
	uvec2 v = uvec2(round(i));
	uint r = Hash_Tchou_2_1_uint(v);
	return (r >> 8) * (1.0 / float(0x00ffffff));
}

vec2 GradientNoise_Deterministic_Float(vec2 p)
{
	float x = Hash_Tchou_2_1_float(p);
	return normalize(vec2(x - floor(x + 0.5), abs(x) - 0.5));
}
float GradientNoise_Deterministic(vec2 uv, vec3 scale)
{
	vec2 p = uv * scale.xy;
	vec2 ip = floor(p);
	vec2 fp = fract(p);
	float d00 = dot(GradientNoise_Deterministic_Float(ip), fp);
	float d01 = dot(GradientNoise_Deterministic_Float(ip + vec2(0, 1)), fp - vec2(0, 1));
	float d10 = dot(GradientNoise_Deterministic_Float(ip + vec2(1, 0)), fp - vec2(1, 0));
	float d11 = dot(GradientNoise_Deterministic_Float(ip + vec2(1, 1)), fp - vec2(1, 1));
	fp = fp * fp * fp * (fp * (fp * 6 - 15) + 10);
	return mix(mix(d00, d01, fp.y), mix(d10, d11, fp.y), fp.x) + 0.5;
})"},
			{"Voronoi_Deterministic", R"(
uvec2 Hash_Tchou_2_2_uint(uvec2 v)
{
    v . y ^= 1103515245U;
    v . x += v . y;
    v . x = v . y;
    v . x ^= v . x >> 5u;
    v . x= 0x27d4eb2du;
    v . y ^= (v . x << 3u);
    return v;
}
void Hash_Tchou_2_2_float(vec2 i, out vec2 o)
{
    uvec2 v = uvec2(ivec2 (round(i)));
    uvec2 r = Hash_Tchou_2_2_uint(v);
    o = (r >> 8) * (1.0 / float (0x00ffffff));
}

vec2 Voronoi_Deterministic_float(vec2 uv, float offset)
{
	Hash_Tchou_2_2_float(uv, uv);
	return vec2(sin(uv.y * offset), cos(uv.x * offset)) * 0.5 + vec2(0.5);
}

float Voronoi_Deterministic(vec2 uv float angleOffset, float cellDensity)
{
	vec2 g = floor(uv * cellDensity);
	vec2 f = fract(uv * cellDensity);
	float t = 8.0;
	vec3 res = vec3(8.0, 0.0, 0.0);

	float out = 0.0;
	float cells = 0.0;

	for(int y = -1, y <= 1; ++y)
	{
		for(int x = -1; x <= 1; ++x)
		{
			vec2 lattice = vec2(x, y);
			vec2 offset = Voronoi_Deterministic_float(lattice + g, angleOffset);
			float d = distance(lattice + offset, f);
			if(d < res.x)
			{
				res = vec3(d, offset.x, offset.y);
				out = res.x;
				cells = res.y;
			}
		}
	}
	return out;
})"}// Supposed to have 2 outs
		};

		std::unordered_map<std::string, cShaderFunc> cShaderFuncsTemplates{
			{"END_COLOR", {"finalCol = %s;", "", ShaderGraphFunc_T::IMMUTABLE, CSHAD_T::NIL, {CSHAD_T::VEC4}}},
			{"END_ROUGHNESS", {"finalRoughness = %s;", "", ShaderGraphFunc_T::IMMUTABLE, CSHAD_T::NIL, {CSHAD_T::FLOAT}}},
			{"END_METALLIC", {"finalMetallic = %s;", "", ShaderGraphFunc_T::IMMUTABLE, CSHAD_T::NIL, {CSHAD_T::FLOAT}}},
			{"END_NORMAL", {"finalNormal = extractNom(%s);", "extractNom", ShaderGraphFunc_T::IMMUTABLE, CSHAD_T::NIL, {CSHAD_T::VEC2}}},
			{"END_EMISSION", {"finalEmission = %s;", "", ShaderGraphFunc_T::IMMUTABLE, CSHAD_T::NIL, {CSHAD_T::VEC3}}},

			{"Vec4_f", {"vec4 %s = vec4(%s, %s, %s, %s);", "", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::VEC4, {CSHAD_T::FLOAT, CSHAD_T::FLOAT, CSHAD_T::FLOAT, CSHAD_T::FLOAT}}},
			
			{"GetR", {"float %s = %s.x;", "", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::FLOAT, {CSHAD_T::VEC4}}},
			{"GetG", {"float %s = %s.y;", "", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::FLOAT, {CSHAD_T::VEC4}}},
			{"GetB", {"float %s = %s.z;", "", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::FLOAT, {CSHAD_T::VEC4}}},
			{"GetA", {"float %s = %s.a;", "", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::FLOAT, {CSHAD_T::VEC4}}},
			{"GetRGB", {"vec3 %s = %s.rgb;", "", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::VEC3, {CSHAD_T::VEC4}}},

			{"SetR", {"vec4 %s = SetV4F(%s, %s, 0);", "SetV4F", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::VEC4, {CSHAD_T::VEC4, CSHAD_T::FLOAT}}},
			{"SetG", {"vec4 %s = SetV4F(%s, %s, 1);", "SetV4F", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::VEC4, {CSHAD_T::VEC4, CSHAD_T::FLOAT}}},
			{"SetB", {"vec4 %s = SetV4F(%s, %s, 2);", "SetV4F", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::VEC4, {CSHAD_T::VEC4, CSHAD_T::FLOAT}}},
			{"SetA", {"vec4 %s = SetV4F(%s, %s, 3);", "SetV4F", ShaderGraphFunc_T::VECTOR_MANIP, CSHAD_T::VEC4, {CSHAD_T::VEC4, CSHAD_T::FLOAT}}},

			{"Sine", {"float %s = sin(%s);", "", ShaderGraphFunc_T::MATH, CSHAD_T::FLOAT,{CSHAD_T::FLOAT}}},
			{"Cosine", {"float %s = cos(%s);", "", ShaderGraphFunc_T::MATH, CSHAD_T::FLOAT,{CSHAD_T::FLOAT}}},
			{"Arctan", {"float %s = atan(%s, %s);", "", ShaderGraphFunc_T::MATH, CSHAD_T::FLOAT,{CSHAD_T::FLOAT, CSHAD_T::FLOAT}}},
			{"Sat", {"vec3 %s = sat_Vec4(%s);", "sat_Vec4", ShaderGraphFunc_T::MATH, CSHAD_T::VEC4,{CSHAD_T::VEC4}}},
			{"Fresnel_f", {"float %s = pow((1.0 - sat_f(dot(normalize(%s), normalize(%s)))), %s);", "sat_f", ShaderGraphFunc_T::MATH, CSHAD_T::FLOAT, {CSHAD_T::VEC3, CSHAD_T::VEC3, CSHAD_T::FLOAT}}},

			{"Add", {"vec4 %s = %s + %s;", "", ShaderGraphFunc_T::MATH, CSHAD_T::VEC4, {CSHAD_T::VEC4, CSHAD_T::VEC4}}},
			{"Subtract", {"vec4 %s = %s - %s;", "", ShaderGraphFunc_T::MATH, CSHAD_T::VEC4, {CSHAD_T::VEC4, CSHAD_T::VEC4}}},
			{"Multiply", {"vec4 %s = %s * %s;", "", ShaderGraphFunc_T::MATH, CSHAD_T::VEC4, {CSHAD_T::VEC4, CSHAD_T::VEC4}}},
			{"Dot", {"float %s = dot(%s, %s);", "", ShaderGraphFunc_T::MATH, CSHAD_T::FLOAT, {CSHAD_T::VEC4, CSHAD_T::VEC4}}},
			{"Cross", {"vec4 %s = cross(%s, %s);", "", ShaderGraphFunc_T::MATH, CSHAD_T::VEC4, {CSHAD_T::VEC4, CSHAD_T::VEC4}}},
			{"Normalize", {"vec4 %s = normalize(%s);", "", ShaderGraphFunc_T::MATH, CSHAD_T::VEC4, {CSHAD_T::VEC4}}},
			{"Length", {"float %s = length(%s);", "", ShaderGraphFunc_T::MATH, CSHAD_T::FLOAT, {CSHAD_T::VEC4}}},
			{"One_Minus_f", {"float %s = 1.f - %s;", "", ShaderGraphFunc_T::MATH, CSHAD_T::FLOAT, {CSHAD_T::FLOAT}}},
			{"Flip_Y_Vec2", {"vec2 %s = flipY(%s);","flipY", ShaderGraphFunc_T::MATH, CSHAD_T::VEC2, {CSHAD_T::VEC2}}},
			
			{"Clamp", {"vec4 %s = clamp(%s, %s, %s);", "", ShaderGraphFunc_T::UTILITIES, CSHAD_T::VEC4, {CSHAD_T::VEC4,CSHAD_T::VEC4,CSHAD_T::VEC4}}},
			{"Min", {"vec4 %s = min(%s, %s);", "", ShaderGraphFunc_T::UTILITIES, CSHAD_T::VEC4, {CSHAD_T::VEC4,CSHAD_T::VEC4}}},
			{"Max", {"vec4 %s = max(%s, %s);", "", ShaderGraphFunc_T::UTILITIES, CSHAD_T::VEC4, {CSHAD_T::VEC4,CSHAD_T::VEC4}}},
			{"Lerp", {"vec4 %s = mix(%s, %s, %s);", "", ShaderGraphFunc_T::UTILITIES, CSHAD_T::VEC4, {CSHAD_T::VEC4,CSHAD_T::VEC4,CSHAD_T::FLOAT}}},
			{"SmoothStep", {"vec4 %s = smoothstep(%s, %s, %s);", "", ShaderGraphFunc_T::UTILITIES, CSHAD_T::VEC4, {CSHAD_T::VEC4,CSHAD_T::VEC4,CSHAD_T::FLOAT}}},
			
			{"sampleTexture", {"vec4 %s = texture(%s, %s);", "", ShaderGraphFunc_T::UTILITIES, CSHAD_T::VEC4, {CSHAD_T::SAMPLER, CSHAD_T::VEC2}}},
			{"sampleTextureOffset", {"vec4 %s = OffsetTexture(%s, %s, %s, %s, %s);", "OffsetTexture", ShaderGraphFunc_T::UTILITIES, CSHAD_T::VEC4, {CSHAD_T::SAMPLER, CSHAD_T::FLOAT,  CSHAD_T::FLOAT, CSHAD_T::FLOAT, CSHAD_T::FLOAT}}},
			{"fRand_Vec2", {"float %s = frand_vec2(%s);", "frand_Vec2", ShaderGraphFunc_T::UTILITIES, CSHAD_T::FLOAT, {CSHAD_T::VEC2}}},
			{"Tiling_And_Offset_Vec2", {"vec2 %s = %s * %s + %s;", "", ShaderGraphFunc_T::UTILITIES, CSHAD_T::VEC2, {CSHAD_T::VEC2, CSHAD_T::VEC2}}},
			{"Gradient_Noise_f", {"float %s = GradientNoise_Deterministic(%s, %s);", "GradientNoise_Deterministic", ShaderGraphFunc_T::UTILITIES, CSHAD_T::FLOAT, {CSHAD_T::VEC2, CSHAD_T::VEC3}}}
		};
		// ----- Inside LoadCShader Func =====
		std::unordered_map<std::string, CSHAD_T> dataIDS
		{
			{"vPos", CSHAD_T::VEC3},
			{"vNom", CSHAD_T::VEC3},
			{"vUV", CSHAD_T::VEC2},
			{"color", CSHAD_T::VEC4},
			{"color2", CSHAD_T::VEC3},
			{"time", CSHAD_T::FLOAT}
		};
		std::unordered_map<CSHAD_T, std::string> cDefaultEmptyVals
		{
			{CSHAD_T::BOOL, "false"},
			{CSHAD_T::INT, "0"},
			{CSHAD_T::UINT, "0"},
			{CSHAD_T::FLOAT, "0.0f"},
			{CSHAD_T::VEC2, "vec2(0.0f)"},
			{CSHAD_T::VEC3, "vec3(0.0f)"},
			{CSHAD_T::VEC4, "vec4(0.0f, 0.0f, 0.0f, 1.0f)"},
			{CSHAD_T::SAMPLER, "textures[0]"}
		};
		uint16_t PairCshad(CSHAD_T f, CSHAD_T s)
		{
			return (static_cast<uint16_t>(f) << 8) | static_cast<uint16_t>(s);
		}
		std::unordered_map<uint16_t, std::string> cTypecast // In(Actual), Out(Wants)
		{
			// 1-1 match
			{PairCshad(CSHAD_T::BOOL,CSHAD_T::BOOL),"%s"},
			{PairCshad(CSHAD_T::INT,CSHAD_T::INT),"%s"},
			{PairCshad(CSHAD_T::UINT,CSHAD_T::UINT),"%s"},
			{PairCshad(CSHAD_T::FLOAT,CSHAD_T::FLOAT),"%s"},
			{PairCshad(CSHAD_T::VEC2,CSHAD_T::VEC2),"%s"},
			{PairCshad(CSHAD_T::VEC3,CSHAD_T::VEC3),"%s"},
			{PairCshad(CSHAD_T::VEC4,CSHAD_T::VEC4),"%s"},
			{PairCshad(CSHAD_T::SAMPLER,CSHAD_T::SAMPLER),"%s"},
			// Promotions
			{PairCshad(CSHAD_T::FLOAT,CSHAD_T::VEC2),"vec2(%s)"},
			{PairCshad(CSHAD_T::FLOAT,CSHAD_T::VEC3),"vec3(%s)"},
			{PairCshad(CSHAD_T::FLOAT,CSHAD_T::VEC4),"vec4(%s)"},

			{PairCshad(CSHAD_T::VEC2,CSHAD_T::VEC3),"vec3(%s, 0.0f)"},
			{PairCshad(CSHAD_T::VEC2,CSHAD_T::VEC4),"vec4(%s, 0.0f, 1.0f)"},

			{PairCshad(CSHAD_T::VEC3,CSHAD_T::VEC4),"vec4(%s, 1.0f)"},
			// Demotion
			{PairCshad(CSHAD_T::VEC4,CSHAD_T::FLOAT),"%s.x"},
			{PairCshad(CSHAD_T::VEC3,CSHAD_T::FLOAT),"%s.x"},
			{PairCshad(CSHAD_T::VEC2,CSHAD_T::FLOAT),"%s.x"},

			{PairCshad(CSHAD_T::VEC4,CSHAD_T::VEC2),"%s.xy"},
			{PairCshad(CSHAD_T::VEC3,CSHAD_T::VEC2),"%s.xy"},

			{PairCshad(CSHAD_T::VEC4,CSHAD_T::VEC3),"%s.rgb"},

			{PairCshad(CSHAD_T::SAMPLER,CSHAD_T::VEC4), "texture(%s, vUV)"},
			{PairCshad(CSHAD_T::SAMPLER,CSHAD_T::VEC3), "texture(%s, vUV).rgb"},
			{PairCshad(CSHAD_T::SAMPLER,CSHAD_T::VEC2), "texture(%s, vUV).xy"},
			{PairCshad(CSHAD_T::SAMPLER,CSHAD_T::FLOAT),"texture(%s, vUV).x"}
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
			if (paramsJson.contains("Textures"))
				for (auto& [name, components] : paramsJson["Textures"].items())
				{
					ShaderParams inParam{};
					inParam.dataType = SP_TYPE::TEXTURE;
					inParam.name = name;
					inParam.baseData = components.get<uint64_t>();
					dataI[name] = CSHAD_T::SAMPLER;
					dataIn.push_back(inParam);
				}
			// Extract Functions
			std::unordered_map<std::string, std::string> fragInclFunctions{};
			std::string fragMainShaderSource{
R"(void CustomCalc(in vec4 color, in vec3 color2, inout vec4 finalCol, inout vec3 finalNormal, inout float finalRoughness, inout float finalMetallic, inout vec3 finalEmission)
{
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
						ss << "float " << dataIn[i].name << " = ExtractFloat(" << i << ");\n";
						break;
					case SliceEngineTypes::CustomShader::SP_TYPE::TEXTURE:
						ss << "sampler2D " << dataIn[i].name << " = textures[ExtractUint(" << i << ")];\n";
						break;
					}
				}
				fragMainShaderSource += ss.str();
			}
			LoadCShaderFunctions(fragMainShaderSource, fragInclFunctions, dataI, cshaderJson["Main"]);

			std::string opaqueInOuts{
R"(#version 460 core
// Custom Shader
#extension GL_ARB_bindless_texture : require

layout (location=0) in vec3 vPos; // In M Space
layout (location=1) in vec3 vNom; // In MV Space
layout (location=2) in vec2 vUV;
layout (location=3) in flat uint vInstance;
layout (location=4) in mat3 TBN;

layout (location=0)	out vec4 fFragColor; // location 0 is default GL_BACK_LEFT color buffer
layout (location=1) out uint fGID;
layout (location=2) out vec3 fPositionData;
layout (location=3) out vec3 fNormalData;
layout (location=4) out vec3 fMetalRoughLightData;
layout (location=5) out vec3 fEmission;
)"};
			std::string translucentInOuts{
R"(#version 460 core
// Custom Shader
#extension GL_ARB_bindless_texture : require

layout (location=0) in vec3 vPos; // In M Space
layout (location=1) in vec3 vNom; // In MV Space
layout (location=2) in vec2 vUV;
layout (location=3) in flat uint vInstance;
layout (location=4) in mat3 TBN;

layout (location=0)	out vec4 fFragColor; // location 0 is default GL_BACK_LEFT color buffer
layout (location=1) out uint fGID;
layout (location=2) out vec4 fEmission;

struct Light{
	vec3 position;
	float uFarPlane;
	vec3 direction;
	int type;
	vec4 color; // rgb + intensity
	int hasShadow;
	int shadowNum;
	int spotShadowNum;
	float pointAngle;
	mat4 VP;
};

layout (std140, binding = 0) uniform lightSpaceBlock
{
	mat4 lightSpaceMtx[16];
};
layout (std140, binding = 1) uniform lights
{
	Light uDirectionLight;
	Light uLight[150];
};

layout (binding = 2) uniform samplerCube uSkyboxTex;
layout (binding = 4) uniform sampler2DArray uShadowTex;
layout (binding = 5) uniform samplerCubeArray 	uShadowCubeMap;

const float PI = 3.14159265358979323846;
const float EPSILON = 0.000001;
// -TODO- Temporary material values
const float biasModifier = 0.5f;
const int isDirectional = 0;
const int isPoint 		= 1;
const int isSpot 		= 2;

uniform mat4 V;
uniform int numLights;
uniform int hasDirectionalLight;
uniform float cascadePlaneDist[16];
uniform int cascadeCnt;
uniform int translucentIDOnly;
uniform float translucentSelectThreshold;
uniform float skyboxLightingPower = 1.0f;
uniform bool willBloom = false;
uniform vec3 uCamPos;

)"};
			std::string fragStart{
R"(
uniform float time;

struct BasicIDat
{
	mat4 mdlMtx;
	uint entityID;
	uint isIgnoreLights; 
	uint col2;
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
			// GLSL Main Func
			std::string opaqueFragEnd{
	R"(
void main(void){

	fPositionData = vPos;
	fNormalData = normalize(vNom);
	vec4 color = vec4(
	float(iDat[vInstance].col >> 24 & 0xFF),
	float(iDat[vInstance].col >> 16 & 0xFF),
	float(iDat[vInstance].col >> 8 & 0xFF),
	float(iDat[vInstance].col & 0xFF)) / float(0xFF);

	vec3 color2 = vec3(
	float(iDat[vInstance].col2 >> 24 & 0xFF),
	float(iDat[vInstance].col2 >> 16 & 0xFF),
	float(iDat[vInstance].col2 >> 8 & 0xFF)) / float(0xFF);

	fFragColor = vec4(0.f);
	float roughness = 0.f;
	float metallic = 0.f;
	fEmission = vec3(0.0f);
	CustomCalc(color, color2, fFragColor, fNormalData, roughness, metallic, fEmission);
 
	if(fFragColor.a < 0.00001f)
		discard;

	fNormalData = normalize(fNormalData);

	fGID = iDat[vInstance].entityID;
	fMetalRoughLightData = vec3(roughness, metallic, float(iDat[vInstance].isIgnoreLights));
})" };

			std::string translucentFragEnd{
R"(
float getShadowMulti(vec3 n, vec3 l, vec3 projCoords, int layer);
float getShadowSideMulti(vec3 n, vec3 l, float dist, int lightIdx);
float getShadowCubeMulti(vec3 n, vec3 l, float viewDist, float dist, int lightIdx);
vec3 microfacetModel(vec3 v, vec3 n, vec3 lightCol, vec3 l, vec3 dif, float rough, float metal);
vec3 GetRandDir(vec3 seed);

void main(void){

	vec3 nom = vNom;
	vec4 color = vec4(
	float(iDat[vInstance].col >> 24 & 0xFF),
	float(iDat[vInstance].col >> 16 & 0xFF),
	float(iDat[vInstance].col >> 8 & 0xFF),
	float(iDat[vInstance].col & 0xFF)) / float(0xFF);

	vec3 color2 = vec3(
	float(iDat[vInstance].col2 >> 24 & 0xFF),
	float(iDat[vInstance].col2 >> 16 & 0xFF),
	float(iDat[vInstance].col2 >> 8 & 0xFF)) / float(0xFF);

	fFragColor = vec4(0.f);
	float roughness = 0.f;
	float metallic = 0.f;
	vec3 emission = vec3(0.0f);
	CustomCalc(color, color2, fFragColor, nom, roughness, metallic, emission);
 
	if(translucentIDOnly == 1 && fFragColor.a < translucentSelectThreshold || fFragColor.a < 0.00001f)
		discard;

    fGID = iDat[vInstance].entityID;
    if(translucentIDOnly == 1)
		return;
	
	fEmission = vec4(emission, fFragColor.a);    

    vec4 dif = fFragColor;
   
	if(any(notEqual(nom, vec3(0.0f))) && iDat[vInstance].isIgnoreLights == 0)
	{
		nom = normalize(nom);

        vec3 skyAmbient = texture(uSkyboxTex, nom).rgb * skyboxLightingPower;
        fFragColor = vec4(dif.rgb * skyAmbient, dif.a);

        // Copies lighting_Frag code
		vec3 v = normalize(-vPos);
		
		if(hasDirectionalLight != 0)
		{
	    	vec4 fragViewSpace = V * vec4(vPos, 1.0f);
	    	float depthVal = abs(fragViewSpace.z);
	    	int layer = -1;
	    	for(int i = 0; i < cascadeCnt; ++i)
	    	{
	    		if(depthVal <= cascadePlaneDist[i])
	    		{
	    			layer = i;
	    			break;
	    		}
	    	}
	    	if(layer == -1)
	    		layer = cascadeCnt - 1;

	    	vec4 vLightPos = lightSpaceMtx[layer] * vec4(vPos, 1.0f);
	    	vec3 projCoords = vLightPos.xyz / vLightPos.w;
	    	projCoords = projCoords * 0.5f + 0.5f;

	    	vec3 finalLighting = vec3(0.0f); // if blocked by shadow

			vec3 l = normalize(-uDirectionLight.direction);// Surface to Light
			float shadow = uDirectionLight.hasShadow * getShadowMulti(nom, l, projCoords, layer);
			finalLighting += (1.0 - shadow) * microfacetModel(v, nom, uDirectionLight.color.rgb * uDirectionLight.color.a, l, dif.rgb, roughness, metallic);
			fFragColor += vec4(finalLighting, 0.0f);
		}
		for(int lightIdx = 0; lightIdx < numLights; ++lightIdx)
        {
    		if(uLight[lightIdx].type == isPoint)
    		{
				vec3 l = uLight[lightIdx].position - uCamPos - vPos; // Surface to Light
				float dist = max(length(l), 0.001);
				vec4 lightCol = uLight[lightIdx].color;
				lightCol.a /= (dist * dist); // Insensity is normalized, so scale up by 100?

				float shadow = uLight[lightIdx].hasShadow * getShadowCubeMulti(nom, l, length(vPos), dist, lightIdx);
				l = l / dist;
				fFragColor += vec4(((1.0 - shadow) * microfacetModel(v, nom, lightCol.rgb * lightCol.a, l, dif.rgb, roughness, metallic)), 0.0f);
    		}
			else if(uLight[lightIdx].type == isSpot)
			{
				vec3 l = uLight[lightIdx].position - uCamPos - vPos;
    			float dist = max(length(l), 0.001);
    			vec3 L = l / dist;
    			vec3 lightDir = normalize(uLight[lightIdx].direction); // Spotlight's forward direction
    			vec3 fragToLight = -L;

				float theta = dot(lightDir, fragToLight);
    			float cutOff = cos(float(uLight[lightIdx].pointAngle) / 2.0);

				float epsilon = 0.05;
    			float intensity = clamp((theta - cutOff) / epsilon, 0.0, 1.0);

				if (intensity > 0.0) {
					vec4 lightCol = uLight[lightIdx].color;
					lightCol.a = lightCol.a / (dist * dist) * intensity;

					float shadow = uLight[lightIdx].hasShadow * getShadowSideMulti(nom, l, dist, lightIdx);
					fFragColor += vec4(((1.0 - shadow) * microfacetModel(v, nom, lightCol.rgb * lightCol.a, L, dif.rgb, roughness, metallic)), 0.0f);
    			}
			}
        }
	}
	//if(!willBloom)
		fFragColor += vec4(fEmission.rgb, 0.0f);
}


float GgxDistribution(float nDotH, float rough)
{
	float alpha2 = rough * rough * rough * rough;
	float d = nDotH * nDotH * (alpha2 - 1.0) + 1.0;
	return alpha2 / (PI * d * d);
}

vec3 SchlickFresnel(float lDotH, vec3 dif, float metal)
{
	vec3 f0 = vec3(0.04); // -TODO- Dielectrics

	f0 = mix(f0, dif, metal);
	return f0 + (1.0 - f0) * pow(clamp(1.0 - lDotH, 0.0, 1.0), 5.0);
}

float GeomSmith(float nDotL, float rough)
{
	float k = (rough + 1.0) * (rough + 1.0) / 8.0;
	float d = nDotL * (1.0 - k) + k;
	return nDotL / d;
}

vec3 microfacetModel(vec3 v, vec3 n, vec3 lightCol, vec3 l, vec3 dif, float rough, float metal)
{
	vec3 h = normalize(v + l);
	float nDotH = clamp(dot(n, h), 0.0, 1.0);
	float vDotH = clamp(dot(v, h), 0.0, 1.0);
	float nDotL = clamp(dot(n, l), 0.0, 1.0);
	float nDotV = abs(dot(n, v)) + 1e-5;

	vec3 F = SchlickFresnel(vDotH, dif, metal);
	vec3 kD = 1.0 - F;
	vec3 specBRDF_nom = GgxDistribution(nDotH, rough) *
					F *
					GeomSmith(nDotL, rough) *
					GeomSmith(nDotV, rough);
	float specBRDF_denom = 4.0 * nDotV * nDotL + 1e-5;
	vec3 specBPDF = specBRDF_nom / specBRDF_denom;
	vec3 diffuseBRDF = kD * dif / PI;
	return (diffuseBRDF + specBPDF) * lightCol * nDotL;
}

float getShadowMulti(vec3 n, vec3 l, vec3 projCoords, int layer)
{
	if(projCoords.z > 1.0)
        return 0.0;
	
	float baseBias = max(0.05 * (1.0 - dot(n, l)), 0.005);
	float bias = baseBias * (cascadePlaneDist[layer] * 0.001);

	// PCF
	float shadow = 0.0;
	vec2 texelSize = 1.0 / vec2(textureSize(uShadowTex, 0));
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(uShadowTex, vec3(projCoords.xy + vec2(x,y) * texelSize, layer)).r;
			shadow += projCoords.z - bias > pcfDepth ? 1.0 : 0.0;
		}
	}

	return shadow / 9.0;
}


// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float getShadowCubeMulti(vec3 n, vec3 l, float viewDist, float dist, int lightIdx)
{
	vec3 fragToLight = -l;
	vec3 L = normalize(l);

	float bias = max(0.05 * (1.0 - dot(n, L)), 0.005) * (uLight[lightIdx].uFarPlane / 20.0);
	float diskRadius = (1.0 + (viewDist / 20.0)) / 25.0;
	
	int samples = 20;
	float shadow = 0.0;

	vec3 noise = GetRandDir(vec3(gl_FragCoord.xy, 0.0));

	for(int i = 0; i < samples; ++i)
	{
		vec3 offset = reflect(gridSamplingDisk[i], normalize(noise));

		float closestDepth = texture(uShadowCubeMap, vec4(fragToLight + offset * diskRadius, float(uLight[lightIdx].shadowNum))).r;
		closestDepth *= uLight[lightIdx].uFarPlane;
		if(dist - bias > closestDepth)
			shadow += 1.0;
	}
	return shadow /= float(samples);
}

vec3 UVToCubeDir(vec2 uv, int face) 
{
    vec2 c = uv * 2.0 - 1.0;
    if (face == 0) return vec3(1.0, -c.y, -c.x);       // +X
    if (face == 1) return vec3(-1.0, -c.y, c.x);       // -X
    if (face == 2) return vec3(c.x, 1.0, c.y);         // +Y
    if (face == 3) return vec3(c.x, -1.0, -c.y);       // -Y
    if (face == 4) return vec3(c.x, -c.y, 1.0);        // +Z
    return vec3(-c.x, -c.y, -1.0);                     // -Z
}

float getShadowSideMulti(vec3 n, vec3 l, float dist, int lightIdx)
{
    vec3 L = normalize(l); 
    
    vec4 vLightPos = uLight[lightIdx].VP * vec4(-l, 1.0);
    
    if (vLightPos.w <= 0.0) return 1.0; 
    
    vec3 projCoords = vLightPos.xyz / vLightPos.w;
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
        return 1.0; 

    float currentDepth = dist / uLight[lightIdx].uFarPlane; 
    
    float bias = max(0.05 * (1.0 - dot(n, L)), 0.005) * (uLight[lightIdx].uFarPlane / 20.0);
    float shadow = 0.0;
    
    vec2 texelSize = 1.0 / vec2(textureSize(uShadowCubeMap, 0).xy); 
    float layer = float(uLight[lightIdx].shadowNum); 
    int face = uLight[lightIdx].spotShadowNum;

    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            vec2 offsetUV = projCoords.xy + vec2(x,y) * texelSize;
            vec3 cubeDir = UVToCubeDir(offsetUV, face);
            
            float pcfDepth = texture(uShadowCubeMap, vec4(cubeDir, layer)).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }

    return shadow / 9.0;
}

vec3 GetRandDir(vec3 seed)
{
	float j = 4096.0 * sin(dot(seed, vec3(17.0, 59.4, 15.0)));
	vec3 r;
	r.z = fract(512.0 * j);
	j *= .125;
	r.x = fract(512.0 * j);
	j *= .125;
	r.y = fract(512.0 * j);
	return r - 0.5;
}
)" };

			// Combine all the texts

			std::string sharedFragCode = fragStart + fragNumExtraElems;
			for (auto& i : fragInclFunctions)
				sharedFragCode += cShaderPredefines.find(i.first)->second + "\n";
			sharedFragCode += fragMainShaderSource;

			std::string opaqueFrag = opaqueInOuts + sharedFragCode + opaqueFragEnd;
			std::string translucentFrag = translucentInOuts + sharedFragCode + translucentFragEnd;
			GLchar const* opq_frag_shader_code[] = { opaqueFrag.c_str() };
			GLchar const* trq_frag_shader_code[] = { translucentFrag.c_str() };
			// -----------------------------------------------------------
			// Start GLSL compiling
			int success;
			char infoLog[512];
			//*********************************************************
			std::string VertexShaderName{"Shaders/deferred_Vert.vert"};
			//*********************************************************
			
			// -----------------------------------------------------------
			// Opaque Frag Shader
			GLuint opaqueFragShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(opaqueFragShader, 1, opq_frag_shader_code, nullptr);
			glCompileShader(opaqueFragShader);
			glGetShaderiv(opaqueFragShader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(opaqueFragShader, 512, nullptr, infoLog);
				SLICE_LOG_WARNING("Unable to compile custom opaque fragment shader:" + std::string(infoLog));

				return {};
			}
			GLuint opqShader = glCreateProgram();
			glAttachShader(opqShader, Core::GetInstance()->GetResourceManager()->get<VertShader>(VertexShaderName).get()->s);
			glAttachShader(opqShader, opaqueFragShader);
			glLinkProgram(opqShader);

			glGetProgramiv(opqShader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(opqShader, 512, nullptr, infoLog);
				SLICE_LOG_WARNING("Link / Compile Custom Shader Failed:" + std::string(infoLog));
				glDeleteShader(opaqueFragShader);
				return {};
			}
			glValidateProgram(opqShader);
			glGetProgramiv(opqShader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(opqShader, 512, nullptr, infoLog);
				SLICE_LOG_WARNING("Validate Custom Shader Failed:" + std::string(infoLog));
				glDeleteShader(opaqueFragShader);
				return {};
			}

			glDeleteShader(opaqueFragShader);

			// -----------------------------------------------------------
			// Translucent Frag Shader
			GLuint translucentFragShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(translucentFragShader, 1, trq_frag_shader_code, nullptr);
			glCompileShader(translucentFragShader);
			glGetShaderiv(translucentFragShader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(translucentFragShader, 512, nullptr, infoLog);
				SLICE_LOG_WARNING("Unable to compile custom translucent fragment shader:" + std::string(infoLog));

				return {};
			}

			GLuint trqShader = glCreateProgram();
			glAttachShader(trqShader, Core::GetInstance()->GetResourceManager()->get<VertShader>(VertexShaderName).get()->s);
			glAttachShader(trqShader, translucentFragShader);
			glLinkProgram(trqShader);

			glGetProgramiv(trqShader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(trqShader, 512, nullptr, infoLog);
				SLICE_LOG_WARNING("Link / Compile Custom Shader Failed:" + std::string(infoLog));
				glDeleteShader(translucentFragShader);
				return {};
			}
			glValidateProgram(trqShader);
			glGetProgramiv(trqShader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(trqShader, 512, nullptr, infoLog);
				SLICE_LOG_WARNING("Validate Custom Shader Failed:" + std::string(infoLog));
				glDeleteShader(translucentFragShader);
				return {};
			}

			glDeleteShader(translucentFragShader);



			return { opqShader, trqShader, dataIn };
		}

		namespace
		{
			struct CShadDependencies
			{
				std::vector<std::string> freesList; // What "I" can free
				std::string funcStr{};
				CSHAD_T type;
				std::list<std::pair<CSHAD_T, std::string>> dependenciesRemaining; // Waiting for the actual type
			};

			bool ReplaceNameFuncString(CShadDependencies& newID, const std::string& name, size_t pos, CSHAD_T in, CSHAD_T actl)
			{
				auto p = newID.funcStr.find("%s");
				if (pos != 0)
				{
					size_t accumulated{ p };
					std::string tempFunction{ newID.funcStr };

					for (size_t i{}; i < pos; ++i)
					{
						tempFunction = tempFunction.substr(p + 1);
						p = tempFunction.find("%s");
						if (p != std::string::npos)
							accumulated += p + 1;
					}
					p = accumulated;
				}
				
				if (p != std::string::npos)
				{
					if (in == actl)
					{
						newID.funcStr.replace(p, 2, name);
						return true;
					}
					else
					{
						auto tc = cTypecast.find(PairCshad(actl, in));
						if (tc != cTypecast.end())
						{
							std::string newName = tc->second;
							auto newNameP = newName.find("%s");
							if (newNameP != std::string::npos)
							{
								newName.replace(newNameP, 2, name);
								newID.funcStr.replace(p, 2, newName);
								return true;
							}
						}
					}

				}
				return false;
			}
		}

		void CustomShader::LoadCShaderFunctions(std::string& ret, std::unordered_map<std::string, std::string>& funcsPre, const std::unordered_map<std::string, CSHAD_T>& defaulParmas, nlohmann::json& in)
		{
			std::map<std::string, CShadDependencies> dependenciesLockedLines;
			std::queue<std::string> toClearLines;

			for (auto& [funcName, funcInstance] : in.items())
			{
				auto funcDetails = cShaderFuncsTemplates.find(funcName)->second;
				// Predefines check for all functions used
				if (funcDetails.opPredefine != "" &&
					cShaderPredefines.find(funcDetails.opPredefine) != cShaderPredefines.end())
					funcsPre[funcDetails.opPredefine] = cShaderPredefines.find(funcDetails.opPredefine)->second;
				// Loop through
				for (auto& [id, dependicies] : funcInstance.items())
				{
					// id - the return id of that line
					std::vector<std::string> dep;
					dependicies.get_to(dep);

					CShadDependencies newID{};
					if (dependenciesLockedLines.find(id) != dependenciesLockedLines.end())
						newID = dependenciesLockedLines[id];
					newID.funcStr = funcDetails.code + "\n";
					newID.type = funcDetails.outType;

					// Name of Variable
					if (funcDetails.outType != CSHAD_T::NIL)
						ReplaceNameFuncString(newID, id, 0, CSHAD_T::NIL, CSHAD_T::NIL);
					else if (dep[0] == "0") // Std out has No Dependencies / Is meant to do nothing / Just DONT Call
						continue;
					// type name = %s;
					size_t numEmpty{};
					for (size_t i{}; i < dep.size(); ++i)
					{
						std::string& depName = dep[i];
						// Dependency Non-existant
						if (depName == "0")
						{
							ReplaceNameFuncString(newID, cDefaultEmptyVals[funcDetails.inIDs[i]], i - numEmpty, CSHAD_T::NIL, CSHAD_T::NIL);
							++numEmpty;
						}
						// Dependency is a default value (all)
						else if (defaulParmas.find(depName) != defaulParmas.end())
						{
							ReplaceNameFuncString(newID, depName, i-numEmpty, funcDetails.inIDs[i], defaulParmas.find(depName)->second);
							++numEmpty;
						}
						// Idk what is the type of the dependency yet
						else
						{
							if (dependenciesLockedLines.find(depName) != dependenciesLockedLines.end())
								dependenciesLockedLines[depName].freesList.push_back(id);
							else
							{
								CShadDependencies othID{};
								othID.freesList.push_back(id);
								dependenciesLockedLines[depName] = othID;
							}
							newID.dependenciesRemaining.push_back({ funcDetails.inIDs[i], depName });
						}
					}
					dependenciesLockedLines[id] = newID;
					if (newID.dependenciesRemaining.empty())
						toClearLines.push(id);
				}
			}
			// Extract Functions
			// Uses toClearLines & dependenciesLockedLines
			while (!toClearLines.empty())
			{
				auto i = toClearLines.front();
				toClearLines.pop();
				auto extractDets = dependenciesLockedLines[i];

				// Unlock all functions being blocked by this ID
				for (auto cl : extractDets.freesList)
				{
					auto fl = dependenciesLockedLines.find(cl);
					if (fl != dependenciesLockedLines.end())
					{
						size_t replaceWhichS = 0;
						for (auto node = fl->second.dependenciesRemaining.begin(); node != fl->second.dependenciesRemaining.end(); node = std::next(node))
						{
							if (node->second == i)
							{
								ReplaceNameFuncString(fl->second, i, replaceWhichS, node->first, extractDets.type);

								fl->second.dependenciesRemaining.erase(node);
								if (fl->second.dependenciesRemaining.empty())
									toClearLines.push(fl->first);

								break;
							}
							++replaceWhichS;
						}
					}
				}
				ret += extractDets.funcStr;
			}
			ret += "if(finalCol.a > 1.0) finalCol.a = 1.0;\n}\n";
			//SLICE_LOG("\nCShaderCode: [\n" + ret + "]\n");
		}

		void CustomShader::LoadDefault()
		{
			std::string opqStr{ R"(#version 460 core
// Custom Shader
#extension GL_ARB_bindless_texture : require

layout (location=0) in vec3 vPos; // In M Space
layout (location=1) in vec3 vNom; // In MV Space
layout (location=2) in vec2 vUV;
layout (location=3) in flat uint vInstance;
layout (location=4) in mat3 TBN;

layout (location=0)	out vec4 fFragColor; // location 0 is default GL_BACK_LEFT color buffer
layout (location=1) out uint fGID;
layout (location=2) out vec3 fPositionData;
layout (location=3) out vec3 fNormalData;
layout (location=4) out vec3 fMetalRoughLightData;
layout (location=5) out vec3 fEmission;

uniform float time;

struct BasicIDat
{
	mat4 mdlMtx;
	uint entityID;
	uint isIgnoreLights; 
	uint col2;
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

int numElem = 6;

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
vec3 extractNom(vec2 v){
	float z = sqrt(1.0 - clamp(v.x * v.x + v.y * v.y, 0.0, 1.0));
    vec3 actlNom = normalize(vec3(v.x, v.y, z));
	return normalize(TBN * (actlNom * 2.0f - 1.0f));
}
vec2 flipY(vec2 n) {return vec2(n.x, 1.f-n.y);}
void CustomCalc(in vec4 color, in vec3 color2, inout vec4 finalCol, inout vec3 finalNormal, inout float finalRoughness, inout float finalMetallic, inout vec3 finalEmission)
{
float EmissionMult = ExtractFloat(0);
sampler2D EmissionMap = textures[ExtractUint(1)];
sampler2D MetallicMap = textures[ExtractUint(2)];
sampler2D NormalMap = textures[ExtractUint(3)];
sampler2D RoughnessMap = textures[ExtractUint(4)];
sampler2D albedo = textures[ExtractUint(5)];
vec2 Node27 = flipY(vUV);
vec4 Node34 = texture(NormalMap, Node27);
vec4 Node38 = texture(albedo, Node27);
vec4 Node42 = texture(MetallicMap, Node27);
vec4 Node46 = texture(RoughnessMap, Node27);
vec4 Node50 = texture(EmissionMap, Node27);
finalNormal = extractNom(Node34.xy);
vec4 Node30 = color * Node38;
finalMetallic = Node42.x;
finalRoughness = Node46.x;
vec4 Node545 = Node50 * vec4(color2, 1.0f);
finalCol = Node30;
vec4 Node74 = Node545 * vec4(EmissionMult);
finalEmission = Node74.rgb;
if(finalCol.a > 1.0) finalCol.a = 1.0;
}

void main(void){

	fPositionData = vPos;
	fNormalData = normalize(vNom);
	vec4 color = vec4(
	float(iDat[vInstance].col >> 24 & 0xFF),
	float(iDat[vInstance].col >> 16 & 0xFF),
	float(iDat[vInstance].col >> 8 & 0xFF),
	float(iDat[vInstance].col & 0xFF)) / float(0xFF);

	vec3 color2 = vec3(
	float(iDat[vInstance].col2 >> 24 & 0xFF),
	float(iDat[vInstance].col2 >> 16 & 0xFF),
	float(iDat[vInstance].col2 >> 8 & 0xFF)) / float(0xFF);

	fFragColor = vec4(0.f);
	float roughness = 0.f;
	float metallic = 0.f;
	fEmission = vec3(0.0f);
	CustomCalc(color, color2, fFragColor, fNormalData, roughness, metallic, fEmission);
 
	if(fFragColor.a < 0.00001f)
		discard;

	fNormalData = normalize(fNormalData);

	fGID = iDat[vInstance].entityID;
	fMetalRoughLightData = vec3(roughness, metallic, float(iDat[vInstance].isIgnoreLights));
})" };
			std::string trqStr{ R"(#version 460 core
// Custom Shader
#extension GL_ARB_bindless_texture : require

layout (location=0) in vec3 vPos; // In M Space
layout (location=1) in vec3 vNom; // In MV Space
layout (location=2) in vec2 vUV;
layout (location=3) in flat uint vInstance;
layout (location=4) in mat3 TBN;

layout (location=0)	out vec4 fFragColor; // location 0 is default GL_BACK_LEFT color buffer
layout (location=1) out uint fGID;
layout (location=2) out vec4 fEmission;

struct Light{
	vec3 position;
	float uFarPlane;
	vec3 direction;
	int type;
	vec4 color; // rgb + intensity
	int hasShadow;
	int shadowNum;
	int spotShadowNum;
	float pointAngle;
	mat4 VP;
};

layout (std140, binding = 0) uniform lightSpaceBlock
{
	mat4 lightSpaceMtx[16];
};
layout (std140, binding = 1) uniform lights
{
	Light uDirectionLight;
	Light uLight[150];
};

layout (binding = 2) uniform samplerCube uSkyboxTex;
layout (binding = 4) uniform sampler2DArray uShadowTex;
layout (binding = 5) uniform samplerCubeArray 	uShadowCubeMap;

const float PI = 3.14159265358979323846;
const float EPSILON = 0.000001;
// -TODO- Temporary material values
const float biasModifier = 0.5f;
const int isDirectional = 0;
const int isPoint 		= 1;
const int isSpot 		= 2;

uniform mat4 V;
uniform int numLights;
uniform int hasDirectionalLight;
uniform float cascadePlaneDist[16];
uniform int cascadeCnt;
uniform int translucentIDOnly;
uniform float translucentSelectThreshold;
uniform float skyboxLightingPower = 1.0f;
uniform bool willBloom = false;
uniform vec3 uCamPos;


uniform float time;

struct BasicIDat
{
	mat4 mdlMtx;
	uint entityID;
	uint isIgnoreLights; 
	uint col2;
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

int numElem = 6;

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
vec3 extractNom(vec2 v){
	float z = sqrt(1.0 - clamp(v.x * v.x + v.y * v.y, 0.0, 1.0));
    vec3 actlNom = normalize(vec3(v.x, v.y, z));
	return normalize(TBN * (actlNom * 2.0f - 1.0f));
}
vec2 flipY(vec2 n) {return vec2(n.x, 1.f-n.y);}
void CustomCalc(in vec4 color, in vec3 color2, inout vec4 finalCol, inout vec3 finalNormal, inout float finalRoughness, inout float finalMetallic, inout vec3 finalEmission)
{
float EmissionMult = ExtractFloat(0);
sampler2D EmissionMap = textures[ExtractUint(1)];
sampler2D MetallicMap = textures[ExtractUint(2)];
sampler2D NormalMap = textures[ExtractUint(3)];
sampler2D RoughnessMap = textures[ExtractUint(4)];
sampler2D albedo = textures[ExtractUint(5)];
vec2 Node27 = flipY(vUV);
vec4 Node34 = texture(NormalMap, Node27);
vec4 Node38 = texture(albedo, Node27);
vec4 Node42 = texture(MetallicMap, Node27);
vec4 Node46 = texture(RoughnessMap, Node27);
vec4 Node50 = texture(EmissionMap, Node27);
finalNormal = extractNom(Node34.xy);
vec4 Node30 = color * Node38;
finalMetallic = Node42.x;
finalRoughness = Node46.x;
vec4 Node545 = Node50 * vec4(color2, 1.0f);
finalCol = Node30;
vec4 Node74 = Node545 * vec4(EmissionMult);
finalEmission = Node74.rgb;
if(finalCol.a > 1.0) finalCol.a = 1.0;
}

float getShadowMulti(vec3 n, vec3 l, vec3 projCoords, int layer);
float getShadowSideMulti(vec3 n, vec3 l, float dist, int lightIdx);
float getShadowCubeMulti(vec3 n, vec3 l, float viewDist, float dist, int lightIdx);
vec3 microfacetModel(vec3 v, vec3 n, vec3 lightCol, vec3 l, vec3 dif, float rough, float metal);
vec3 GetRandDir(vec3 seed);

void main(void){

	vec3 nom = vNom;
	vec4 color = vec4(
	float(iDat[vInstance].col >> 24 & 0xFF),
	float(iDat[vInstance].col >> 16 & 0xFF),
	float(iDat[vInstance].col >> 8 & 0xFF),
	float(iDat[vInstance].col & 0xFF)) / float(0xFF);

	vec3 color2 = vec3(
	float(iDat[vInstance].col2 >> 24 & 0xFF),
	float(iDat[vInstance].col2 >> 16 & 0xFF),
	float(iDat[vInstance].col2 >> 8 & 0xFF)) / float(0xFF);

	fFragColor = vec4(0.f);
	float roughness = 0.f;
	float metallic = 0.f;
	vec3 emission = vec3(0.0f);
	CustomCalc(color, color2, fFragColor, nom, roughness, metallic, emission);
 
	if(translucentIDOnly == 1 && fFragColor.a < translucentSelectThreshold || fFragColor.a < 0.00001f)
		discard;

    fGID = iDat[vInstance].entityID;
    if(translucentIDOnly == 1)
		return;
	
	fEmission = vec4(emission, fFragColor.a);    

    vec4 dif = fFragColor;
   
	if(any(notEqual(nom, vec3(0.0f))) && iDat[vInstance].isIgnoreLights == 0)
	{
		nom = normalize(nom);

        vec3 skyAmbient = texture(uSkyboxTex, nom).rgb * skyboxLightingPower;
        fFragColor = vec4(dif.rgb * skyAmbient, dif.a);

        // Copies lighting_Frag code
		vec3 v = normalize(-vPos);
		
		if(hasDirectionalLight != 0)
		{
	    	vec4 fragViewSpace = V * vec4(vPos, 1.0f);
	    	float depthVal = abs(fragViewSpace.z);
	    	int layer = -1;
	    	for(int i = 0; i < cascadeCnt; ++i)
	    	{
	    		if(depthVal <= cascadePlaneDist[i])
	    		{
	    			layer = i;
	    			break;
	    		}
	    	}
	    	if(layer == -1)
	    		layer = cascadeCnt - 1;

	    	vec4 vLightPos = lightSpaceMtx[layer] * vec4(vPos, 1.0f);
	    	vec3 projCoords = vLightPos.xyz / vLightPos.w;
	    	projCoords = projCoords * 0.5f + 0.5f;

	    	vec3 finalLighting = vec3(0.0f); // if blocked by shadow

			vec3 l = normalize(-uDirectionLight.direction);// Surface to Light
			float shadow = uDirectionLight.hasShadow * getShadowMulti(nom, l, projCoords, layer);
			finalLighting += (1.0 - shadow) * microfacetModel(v, nom, uDirectionLight.color.rgb * uDirectionLight.color.a, l, dif.rgb, roughness, metallic);
			fFragColor += vec4(finalLighting, 0.0f);
		}
		for(int lightIdx = 0; lightIdx < numLights; ++lightIdx)
        {
    		if(uLight[lightIdx].type == isPoint)
    		{
				vec3 l = uLight[lightIdx].position - uCamPos - vPos; // Surface to Light
				float dist = max(length(l), 0.001);
				vec4 lightCol = uLight[lightIdx].color;
				lightCol.a /= (dist * dist); // Insensity is normalized, so scale up by 100?

				float shadow = uLight[lightIdx].hasShadow * getShadowCubeMulti(nom, l, length(vPos), dist, lightIdx);
				l = l / dist;
				fFragColor += vec4(((1.0 - shadow) * microfacetModel(v, nom, lightCol.rgb * lightCol.a, l, dif.rgb, roughness, metallic)), 0.0f);
    		}
			else if(uLight[lightIdx].type == isSpot)
			{
				vec3 l = uLight[lightIdx].position - uCamPos - vPos;
    			float dist = max(length(l), 0.001);
    			vec3 L = l / dist;
    			vec3 lightDir = normalize(uLight[lightIdx].direction); // Spotlight's forward direction
    			vec3 fragToLight = -L;

				float theta = dot(lightDir, fragToLight);
    			float cutOff = cos(float(uLight[lightIdx].pointAngle) / 2.0);

				float epsilon = 0.05;
    			float intensity = clamp((theta - cutOff) / epsilon, 0.0, 1.0);

				if (intensity > 0.0) {
					vec4 lightCol = uLight[lightIdx].color;
					lightCol.a = lightCol.a / (dist * dist) * intensity;

					float shadow = uLight[lightIdx].hasShadow * getShadowSideMulti(nom, l, dist, lightIdx);
					fFragColor += vec4(((1.0 - shadow) * microfacetModel(v, nom, lightCol.rgb * lightCol.a, L, dif.rgb, roughness, metallic)), 0.0f);
    			}
			}
        }
	}
	//if(!willBloom)
		fFragColor += vec4(fEmission.rgb, 0.0f);
}


float GgxDistribution(float nDotH, float rough)
{
	float alpha2 = rough * rough * rough * rough;
	float d = nDotH * nDotH * (alpha2 - 1.0) + 1.0;
	return alpha2 / (PI * d * d);
}

vec3 SchlickFresnel(float lDotH, vec3 dif, float metal)
{
	vec3 f0 = vec3(0.04); // -TODO- Dielectrics

	f0 = mix(f0, dif, metal);
	return f0 + (1.0 - f0) * pow(clamp(1.0 - lDotH, 0.0, 1.0), 5.0);
}

float GeomSmith(float nDotL, float rough)
{
	float k = (rough + 1.0) * (rough + 1.0) / 8.0;
	float d = nDotL * (1.0 - k) + k;
	return nDotL / d;
}

vec3 microfacetModel(vec3 v, vec3 n, vec3 lightCol, vec3 l, vec3 dif, float rough, float metal)
{
	vec3 h = normalize(v + l);
	float nDotH = clamp(dot(n, h), 0.0, 1.0);
	float vDotH = clamp(dot(v, h), 0.0, 1.0);
	float nDotL = clamp(dot(n, l), 0.0, 1.0);
	float nDotV = abs(dot(n, v)) + 1e-5;

	vec3 F = SchlickFresnel(vDotH, dif, metal);
	vec3 kD = 1.0 - F;
	vec3 specBRDF_nom = GgxDistribution(nDotH, rough) *
					F *
					GeomSmith(nDotL, rough) *
					GeomSmith(nDotV, rough);
	float specBRDF_denom = 4.0 * nDotV * nDotL + 1e-5;
	vec3 specBPDF = specBRDF_nom / specBRDF_denom;
	vec3 diffuseBRDF = kD * dif / PI;
	return (diffuseBRDF + specBPDF) * lightCol * nDotL;
}

float getShadowMulti(vec3 n, vec3 l, vec3 projCoords, int layer)
{
	if(projCoords.z > 1.0)
        return 0.0;
	
	float baseBias = max(0.05 * (1.0 - dot(n, l)), 0.005);
	float bias = baseBias * (cascadePlaneDist[layer] * 0.001);

	// PCF
	float shadow = 0.0;
	vec2 texelSize = 1.0 / vec2(textureSize(uShadowTex, 0));
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(uShadowTex, vec3(projCoords.xy + vec2(x,y) * texelSize, layer)).r;
			shadow += projCoords.z - bias > pcfDepth ? 1.0 : 0.0;
		}
	}

	return shadow / 9.0;
}


// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float getShadowCubeMulti(vec3 n, vec3 l, float viewDist, float dist, int lightIdx)
{
	vec3 fragToLight = -l;
	vec3 L = normalize(l);

	float bias = max(0.05 * (1.0 - dot(n, L)), 0.005) * (uLight[lightIdx].uFarPlane / 20.0);
	float diskRadius = (1.0 + (viewDist / 20.0)) / 25.0;
	
	int samples = 20;
	float shadow = 0.0;

	vec3 noise = GetRandDir(vec3(gl_FragCoord.xy, 0.0));

	for(int i = 0; i < samples; ++i)
	{
		vec3 offset = reflect(gridSamplingDisk[i], normalize(noise));

		float closestDepth = texture(uShadowCubeMap, vec4(fragToLight + offset * diskRadius, float(uLight[lightIdx].shadowNum))).r;
		closestDepth *= uLight[lightIdx].uFarPlane;
		if(dist - bias > closestDepth)
			shadow += 1.0;
	}
	return shadow /= float(samples);
}

vec3 UVToCubeDir(vec2 uv, int face) 
{
    vec2 c = uv * 2.0 - 1.0;
    if (face == 0) return vec3(1.0, -c.y, -c.x);       // +X
    if (face == 1) return vec3(-1.0, -c.y, c.x);       // -X
    if (face == 2) return vec3(c.x, 1.0, c.y);         // +Y
    if (face == 3) return vec3(c.x, -1.0, -c.y);       // -Y
    if (face == 4) return vec3(c.x, -c.y, 1.0);        // +Z
    return vec3(-c.x, -c.y, -1.0);                     // -Z
}

float getShadowSideMulti(vec3 n, vec3 l, float dist, int lightIdx)
{
    vec3 L = normalize(l); 
    
    vec4 vLightPos = uLight[lightIdx].VP * vec4(-l, 1.0);
    
    if (vLightPos.w <= 0.0) return 1.0; 
    
    vec3 projCoords = vLightPos.xyz / vLightPos.w;
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
        return 1.0; 

    float currentDepth = dist / uLight[lightIdx].uFarPlane; 
    
    float bias = max(0.05 * (1.0 - dot(n, L)), 0.005) * (uLight[lightIdx].uFarPlane / 20.0);
    float shadow = 0.0;
    
    vec2 texelSize = 1.0 / vec2(textureSize(uShadowCubeMap, 0).xy); 
    float layer = float(uLight[lightIdx].shadowNum); 
    int face = uLight[lightIdx].spotShadowNum;

    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            vec2 offsetUV = projCoords.xy + vec2(x,y) * texelSize;
            vec3 cubeDir = UVToCubeDir(offsetUV, face);
            
            float pcfDepth = texture(uShadowCubeMap, vec4(cubeDir, layer)).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }

    return shadow / 9.0;
}

vec3 GetRandDir(vec3 seed)
{
	float j = 4096.0 * sin(dot(seed, vec3(17.0, 59.4, 15.0)));
	vec3 r;
	r.z = fract(512.0 * j);
	j *= .125;
	r.x = fract(512.0 * j);
	j *= .125;
	r.y = fract(512.0 * j);
	return r - 0.5;
}
)" };

			GLchar const* opq_frag_shader_code[] = { opqStr.c_str() };
			GLchar const* trq_frag_shader_code[] = { trqStr.c_str() };
			// -----------------------------------------------------------
			// Start GLSL compiling
			int success;
			char infoLog[512];
			//*********************************************************
			std::string VertexShaderName{ "Shaders/deferred_Vert.vert" };
			//*********************************************************

			// -----------------------------------------------------------
			// Opaque Frag Shader
			GLuint opaqueFragShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(opaqueFragShader, 1, opq_frag_shader_code, nullptr);
			glCompileShader(opaqueFragShader);
			glGetShaderiv(opaqueFragShader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(opaqueFragShader, 512, nullptr, infoLog);
				SLICE_LOG_WARNING("Unable to compile custom opaque fragment shader:" + std::string(infoLog));

				return;
			}
			GLuint opqShader = glCreateProgram();
			glAttachShader(opqShader, Core::GetInstance()->GetResourceManager()->get<VertShader>(VertexShaderName).get()->s);
			glAttachShader(opqShader, opaqueFragShader);
			glLinkProgram(opqShader);

			glGetProgramiv(opqShader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(opqShader, 512, nullptr, infoLog);
				SLICE_LOG_WARNING("Link / Compile Custom Shader Failed:" + std::string(infoLog));
				glDeleteShader(opaqueFragShader);
				return;
			}
			glValidateProgram(opqShader);
			glGetProgramiv(opqShader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(opqShader, 512, nullptr, infoLog);
				SLICE_LOG_WARNING("Validate Custom Shader Failed:" + std::string(infoLog));
				glDeleteShader(opaqueFragShader);
				return;
			}

			glDeleteShader(opaqueFragShader);

			// -----------------------------------------------------------
			// Translucent Frag Shader
			GLuint translucentFragShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(translucentFragShader, 1, trq_frag_shader_code, nullptr);
			glCompileShader(translucentFragShader);
			glGetShaderiv(translucentFragShader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(translucentFragShader, 512, nullptr, infoLog);
				SLICE_LOG_WARNING("Unable to compile custom translucent fragment shader:" + std::string(infoLog));

				return;
			}

			GLuint trqShader = glCreateProgram();
			glAttachShader(trqShader, Core::GetInstance()->GetResourceManager()->get<VertShader>(VertexShaderName).get()->s);
			glAttachShader(trqShader, translucentFragShader);
			glLinkProgram(trqShader);

			glGetProgramiv(trqShader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(trqShader, 512, nullptr, infoLog);
				SLICE_LOG_WARNING("Link / Compile Custom Shader Failed:" + std::string(infoLog));
				glDeleteShader(translucentFragShader);
				return;
			}
			glValidateProgram(trqShader);
			glGetProgramiv(trqShader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(trqShader, 512, nullptr, infoLog);
				SLICE_LOG_WARNING("Validate Custom Shader Failed:" + std::string(infoLog));
				glDeleteShader(translucentFragShader);
				return;
			}

			glDeleteShader(translucentFragShader);

			opaqueS = opqShader;
			translucentS = trqShader;
			dataIn.clear();
			dataIn.push_back({"EmissionMult", 1.0f, SP_TYPE::FLOAT});
			auto blackTexture = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Texture>("Textures/Black512.png");
			if(blackTexture.IsValid())
				dataIn.push_back({"EmissionMap", blackTexture.getGUID().GetGUID(), SP_TYPE::TEXTURE});
			else
				dataIn.push_back({"EmissionMap", DefaultResourceIDs::COLOR_DEADED_DEFAULT, SP_TYPE::TEXTURE});
			dataIn.push_back({"MetallicMap", DefaultResourceIDs::COLOR_DEADED_DEFAULT, SP_TYPE::TEXTURE});
			dataIn.push_back({"NormalMap", DefaultResourceIDs::COLOR_NORMAL_DEFAULT, SP_TYPE::TEXTURE});
			dataIn.push_back({"RoughnessMap", DefaultResourceIDs::COLOR_DEADED_DEFAULT, SP_TYPE::TEXTURE});
			dataIn.push_back({"albedo", DefaultResourceIDs::COLOR_DEADED_DEFAULT, SP_TYPE::TEXTURE});
		}

		void CustomShader::DestroyCShader()
		{
			glDeleteShader(opaqueS);
			opaqueS = 0;
			glDeleteShader(translucentS);
			translucentS = 0;
			dataIn.clear();
		}
#pragma endregion
}
}