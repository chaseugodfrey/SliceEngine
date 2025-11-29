#version 460 core

layout (location=0)	out vec4	fFragColor; // location 0 is default GL_BACK_LEFT color buffer

uniform vec4 uColor;

void main(void){
	fFragColor = uColor;
}
