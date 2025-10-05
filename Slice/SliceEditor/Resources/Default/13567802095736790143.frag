#version 460 core

layout (location=0)	out vec4	fFragColor; // location 0 is default GL_BACK_LEFT color buffer

void main(void){
	fFragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
