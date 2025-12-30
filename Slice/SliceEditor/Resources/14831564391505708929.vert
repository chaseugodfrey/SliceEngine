#version 460 core

layout (location=0) in vec3 aPos;

uniform mat4 M; // model transform matrix
uniform mat4 uLightMtx; // Shadow Transform Matrix

void main(void){
	gl_Position = uLightMtx * M * vec4(aPos, 1.0f);
}