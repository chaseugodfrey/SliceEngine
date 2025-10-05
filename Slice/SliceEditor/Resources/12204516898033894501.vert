#version 460 core

layout (location=0) in vec3 aVertexPosition;
layout (location=1)	in vec3	aNom;
layout (location=2)	in vec2	aTex;

layout (location=0) out vec3 vPos;
layout (location=1) out vec3 vNom;
layout (location=2) out vec2 vTexCoord;
layout (location=3) out flat uint vGID;

uniform int	 uPass;
uniform uint aGID;
uniform mat4 M; // model transform matrix
uniform mat4 V; // View transform matrix
uniform mat4 P; // Perspective transform matrix

/***************************************************
* Out: vPos, vNom
*****************************************************/
void Pass0(void){
	mat4 VM	= V * M;
	mat3 N = mat3(vec3(VM[0]), vec3(VM[1]), vec3(VM[2]));
	
	vec4 posInView = VM * vec4(aVertexPosition, 1.0);

	vPos = posInView.xyz;
	vNom = normalize(N * aNom);
	vTexCoord	= aTex;
	vGID = aGID;
	gl_Position	= P * posInView;
}

/***************************************************
* Out: aTex
*****************************************************/
void Pass1(void){
	vTexCoord	= aTex;
	gl_Position	= vec4(aVertexPosition * 2.f, 1.0);
}

void main(void){
	if		(uPass == 0) Pass0();
	else if	(uPass == 1) Pass1();
}