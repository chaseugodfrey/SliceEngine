#version 460 core
//pointShadow_Frag
layout (location=0)	in vec4 gFragPos;

uniform float uFarPlane;

void main(void){
	float dist = length(gFragPos.xyz);
	dist = dist / uFarPlane;

	gl_FragDepth = dist;
}