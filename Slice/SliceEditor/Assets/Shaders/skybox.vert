#version 460 core

layout (location = 0) in vec3 aPos;

uniform mat4 V;
uniform mat4 P;
uniform mat4 M;

layout (location=0) out vec3 vTexCoord;

void main(void){
	gl_Position = P * V * M * vec4(aPos, 1.0);
	vTexCoord = aPos;
}
