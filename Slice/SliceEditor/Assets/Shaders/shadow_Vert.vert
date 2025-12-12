#version 460 core

layout (location=0) in vec3 aPos;

struct BasicInstanceData
{
	mat4 mdlMtx;
	uvec4 mat;
};

layout(binding=1, std430) readonly buffer ssbo1
{
	BasicInstanceData iDat[];
};

uniform mat4 uLightMtx; // Shadow Transform Matrix

void main(void){
	gl_Position = uLightMtx * iDat[gl_InstanceID].mdlMtx * vec4(aPos, 1.0f);
}