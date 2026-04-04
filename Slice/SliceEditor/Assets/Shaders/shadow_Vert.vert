#version 460 core
#extension GL_ARB_shader_viewport_layer_array : require

layout (location=0) in vec3 aPos;

struct BasicIDat
{
	mat4 mdlMtx;
	uint entityID;
	uint isIgnoreLights; 
	uint col2;
	uint col;
};

layout (std140, binding = 0) uniform lightSpaceBlock
{
	mat4 lightSpaceMtx[16];
};
layout(binding=1, std430) readonly buffer ssbo1
{
	BasicIDat iDat[];
};

uniform int uNumCopies = 1;

void main(void){
	int instanceID = gl_InstanceID / uNumCopies;
	int layerIdx = gl_InstanceID % uNumCopies;

	gl_Layer = layerIdx;

	gl_Position = lightSpaceMtx[layerIdx] * iDat[instanceID].mdlMtx * vec4(aPos, 1.0f);
}