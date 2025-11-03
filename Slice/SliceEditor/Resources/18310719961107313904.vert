#version 460 core

layout (location=0) in vec3 aVertexPosition;

uniform mat4 M; // model transform matrix
uniform mat4 V; // View transform matrix
uniform mat4 P; // Perspective transform matrix

void main(void){

	//gl_Position	=  P * V * M * vec4(aVertexPosition, 1.0);
	gl_Position	=  P * V * vec4(aVertexPosition, 1.0);
}