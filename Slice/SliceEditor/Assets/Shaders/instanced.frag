#version 460 core

layout (location=0)	out vec4	fFragColor; // location 0 is default GL_BACK_LEFT color buffer

void main(void){
	//vec4 col = vec4(vTexCoord,1.f);

	//col = texture(uTex, vec3(vTexCoord, vID.y) ) * col;

	fFragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
