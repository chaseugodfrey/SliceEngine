#version 460 core

layout (location=0) in vec3	aVertexPosition;
layout (location=2)	in vec2	aTex;

layout (location=0) out vec2 vTex;

struct InstanceData
{
	mat4 mdlMtx;
	ivec4 mat;
};

layout(binding=2, std430) readonly buffer ssbo1
{
	InstanceData iDat[];
};

uniform mat4 V;
uniform mat4 P;

void main(void){
	gl_Position	= P * V * iDat[gl_InstanceID].mdlMtx * vec4(aVertexPosition, 1.0);
	vTex	= aTex;
}