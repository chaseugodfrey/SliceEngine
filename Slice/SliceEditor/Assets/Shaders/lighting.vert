#version 460 core

struct Light{
	vec3 position;
	vec3 direction;
	vec4 color; // rgb + intensity
	int type;
};

layout (location=0) in vec3 aVertexPosition;

uniform mat4 M; // model transform matrix
uniform mat4 V; // View transform matrix
uniform mat4 P; // Perspective transform matrix
uniform Light uLight;

const int isDirectional = 1;
const int isPoint 		= 2;
const int isSpot 		= 3;

void main(void){
	if(uLight.type == isDirectional)
		gl_Position = vec4(aVertexPosition * 2.0, 1.0);
	else
		gl_Position	= P * V * M * vec4(aVertexPosition, 1.0);
}