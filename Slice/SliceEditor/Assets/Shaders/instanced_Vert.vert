#version 460 core

layout (location=0) in vec3	aVertexPosition;

struct InstanceData
{
	mat4 mdlMtx;
};

layout(binding=3, std430) readonly buffer ssbo1
{
	InstanceData iDat[];
};

uniform mat4 V; // View transform matrix
uniform mat4 P; // Perspective transform matrix

void main(void){
	gl_Position	= P * V * iDat[gl_InstanceID].mdlMtx * vec4(aVertexPosition, 1.0);
}