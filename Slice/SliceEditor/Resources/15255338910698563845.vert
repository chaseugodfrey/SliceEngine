#version 460 core

layout (location=0) in vec3 aVertexPosition;
layout (location=2) in vec2 aTextureCoord;

layout (location=0) out vec2 vTextureCoord;

uniform mat4 canvas_to_ndc;
uniform mat4 M;

/*
struct InstanceData{
	mat4 pos;
	vec4 color;
	//ivec4 rect_eid_depth;   //dont really need this additional info yet
};

layout(std430, binding = 1) readonly buffer InstanceBuffer {
    InstanceData instance_data[];
};
*/

void main(void){
	vTextureCoord 	= aTextureCoord;

	gl_Position	=  canvas_to_ndc * M * vec4(aVertexPosition, 1.0);
	//gl_Position = vec4(aVertexPosition, 1.0);
}