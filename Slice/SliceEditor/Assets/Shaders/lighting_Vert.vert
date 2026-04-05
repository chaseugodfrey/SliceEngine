#version 460 core

layout (location=0) in vec3 aVertexPosition;

void main(void){
	gl_Position = vec4(aVertexPosition * 2.0, 1.0);
}