#version 460 core
//particles_Frag
layout (location=0) in vec2 vTex;

layout (location=0)	out vec4 fFragColor;

layout (binding=0) uniform sampler2D uTex;

void main(void){
	fFragColor = texture(uTex, vTex);
}
