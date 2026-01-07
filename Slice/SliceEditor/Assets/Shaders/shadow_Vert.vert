#version 460 core

layout (location=0) in vec3 aPos;

struct BasicIDat
{
	mat4 mdlMtx;
	uint entityID;
	uint textureID; 
	uint colRG;
	uint colBA;
};

layout(binding=1, std430) readonly buffer ssbo1
{
	BasicIDat iDat[];
};

void main(void){
	gl_Position = iDat[gl_InstanceID].mdlMtx * vec4(aPos, 1.0f);
}