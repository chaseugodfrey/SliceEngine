#version 460 core

layout (location = 0) in vec3 aPos;

uniform mat4 V;
uniform mat4 P;

layout (location=0) out vec3 vTexCoord;

void main(void){
	mat4 viewNoTrans = mat4(mat3(V));
	gl_Position = P * viewNoTrans * vec4(aPos, 1.0);
	vTexCoord = aPos;
}
