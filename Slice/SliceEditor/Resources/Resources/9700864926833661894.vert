#version 460 core

layout (location=0) in vec3 aPos;

uniform mat4 M; // model transform matrix

void main(void){
	gl_Position = M * vec4(aPos, 1.0f);
}