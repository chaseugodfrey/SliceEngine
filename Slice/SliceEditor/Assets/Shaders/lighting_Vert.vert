#version 460 core

struct Light{
	vec3 position;
	float uFarPlane;
	vec3 direction;
	int type;
	vec4 color; // rgb + intensity
};

layout (location=0) in vec3 aVertexPosition;

uniform mat4 M; // model transform matrix
uniform mat4 V; // View transform matrix
uniform mat4 P; // Perspective transform matrix
uniform int lightIdx;

layout (std140, binding = 1) uniform lights
{
	Light uLight[11];
};

const int isDirectional = 0;
const int isPoint 		= 1;
const int isSpot 		= 2;

void main(void){
	if(uLight[lightIdx].type == isDirectional)
		gl_Position = vec4(aVertexPosition * 2.0, 1.0);
	else
		gl_Position	= P * V * M * vec4(aVertexPosition, 1.0);
}