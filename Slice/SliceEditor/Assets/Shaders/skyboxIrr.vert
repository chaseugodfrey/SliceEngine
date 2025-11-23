#version 460 core

layout (location = 0) in vec3 aPos;

uniform mat4 V;
uniform mat4 P;

layout (location=0) out vec3 vTexCoord;

void main(void){
	vTexCoord = aPos;

	mat4 rotView = mat4(mat3(V));
	vec4 clipPos = P * rotView * vec4(aPos, 1.0);

	gl_Position = clipPos.xyww;
}
