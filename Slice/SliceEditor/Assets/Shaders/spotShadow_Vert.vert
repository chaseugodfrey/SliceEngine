#version 460 core

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

void main(void){
	gFragPos = uVP * iDat[gl_InstanceID].mdlMtx * vec4(aPos, 1.0f);
	gl_Position = gFragPos;
}