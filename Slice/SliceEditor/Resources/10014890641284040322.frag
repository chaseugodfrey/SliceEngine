#version 460 core
//pointShadow_Frag
layout (location=0)	in vec4 gFragPos;

uniform vec3 uLightPos;
uniform float uFarPlane;

void main(void){
	float dist = length(gFragPos.xyz - uLightPos);
	dist = dist / uFarPlane;

	gl_FragDepth = dist;
}