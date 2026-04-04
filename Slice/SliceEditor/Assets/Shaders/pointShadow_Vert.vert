#version 460 core
#extension GL_ARB_shader_viewport_layer_array : require

layout (location=0) in vec3 aPos;
layout (location=0)	out vec4 gFragPos; // FragPos from GS (output per emitvertex)

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
uniform mat4 uShadowMat[6];
uniform int uLightIdx;
uniform int uNumCopies = 6;

void main(void){
	int instanceID = gl_InstanceID / uNumCopies;
    int faceIdx = gl_InstanceID % uNumCopies;

	gl_Layer = (uLightIdx * 6) + faceIdx;
	vec4 worldPos = iDat[instanceID].mdlMtx * vec4(aPos, 1.0f);

	gFragPos = worldPos;

	gl_Position = uShadowMat[faceIdx] * worldPos;
}