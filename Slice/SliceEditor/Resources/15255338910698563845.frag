#version 460 core
layout (location=0)	in vec2		vTexCoord;

layout (location=0)	out vec4	fFragColor; // location 0 is default GL_BACK_LEFT color buffer

uniform vec4 rgba;
uniform sampler2D tex2d;

void main(void){

	fFragColor = texture(tex2d, vTexCoord) * rgba;
	//fFragColor = vec4(1.f,0.f,0.f,1.f);
}
