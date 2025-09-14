#version 460 core

layout (location=0)	in vec3		vTexCoord;

layout (location=0)	out vec4	fFragColor; // location 0 is default GL_BACK_LEFT color buffer

layout (binding = 0) uniform sampler2DArray 	uTex;

void main(void){
	vec4 col = vec4(vTexCoord,1.f);

	//col = texture(uTex, vec3(vTexCoord, vID.y) ) * col;

	fFragColor = col;
}
