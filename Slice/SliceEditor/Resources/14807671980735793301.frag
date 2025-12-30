#version 460 core

layout (location=0) in vec3 vTexCoord;

layout (location=0)	out vec4 fFragColor;

layout (binding = 0) uniform samplerCube uCubemapTex;

void main(void){
	fFragColor = texture(uCubemapTex, vTexCoord);
}
