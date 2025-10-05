#version 460 core

layout (location=0) in vec3		aVertexPosition;
layout (location=12) in mat4	iMat;

layout (location=0) out vec3 vTexCoord;

uniform mat4 V; // View transform matrix
uniform mat4 P; // Perspective transform matrix

void main(void){

	gl_Position	= P * V * iMat * vec4(aVertexPosition, 1.0);
	vTexCoord	= aVertexPosition;
}