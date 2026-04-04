#version 460 core
#extension GL_ARB_shader_viewport_layer_array : enable
#extension GL_AMD_vertex_shader_layer : enable
layout (location=0) in vec3 aPos;

layout (location=0)	out vec4 gFragPos;

struct BasicIDat
{
	mat4 mdlMtx;
	uint entityID;
	uint isIgnoreLights; 
	uint col2;
	uint col;
};
layout(binding=1, std430) readonly buffer ssbo1
{
	BasicIDat iDat[];
};

uniform mat4 uVP;
uniform int uLightIdx;

void main(void){
	gFragPos = uVP * iDat[gl_InstanceID].mdlMtx * vec4(aPos, 1.0f);
	gl_Position = gFragPos;
	gl_Layer = uLightIdx;
}