#version 460 core

layout (location=0) in vec3 aVertexPosition;
layout (location=1)	in vec3	aNom;
layout (location=2)	in vec2	aTex;

layout (location=0) out vec3 vPos; // World Space
layout (location=1) out vec3 vNom;
layout (location=2) out vec2 vTex;
layout (location=3) out flat uint vGID;

uniform uint aGID;
uniform mat4 M; // model transform matrix
uniform mat4 V; // View transform matrix
uniform mat4 P; // Perspective transform matrix

void main(void){
	mat3 N = transpose(inverse(mat3(M)));
	
	vec4 posInWorld = M * vec4(aVertexPosition, 1.0);

	vPos = posInWorld.xyz;
	vNom = normalize(N * aNom);
	vTex = aTex;
	vGID = aGID;
	gl_Position	= P * V * posInWorld;
}