#version 460 core

layout (location=0) in vec3	aVertexPosition;

struct BasicIDat
{
	mat4 mdlMtx;
	uint entityID;
	uint isIgnoreLights; 
	uint col2;
	uint col;
};

layout(binding=1, std430) readonly buffer ssbo1
{
	BasicIDat iDat[];
};

uniform mat4 V; // View transform matrix
uniform mat4 P; // Perspective transform matrix

void main(void){
	gl_Position	= P * V * iDat[gl_InstanceID].mdlMtx * vec4(aVertexPosition, 1.0);
}