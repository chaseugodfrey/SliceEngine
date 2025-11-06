#version 460 core

layout (location=0) in vec3		aVertexPosition;

layout (location=0) out vec3 vTexCoord;

layout(binding=2, std430) readonly buffer ssbo1
{
	mat4 mdlMtx[];
};

uniform mat4 V; // View transform matrix
uniform mat4 P; // Perspective transform matrix

void main(void){

	gl_Position	= P * V * mdlMtx[gl_InstanceID] * vec4(aVertexPosition, 1.0);
	vTexCoord	= aVertexPosition;
}