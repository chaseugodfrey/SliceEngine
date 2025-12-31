#version 460 core
//deferred_Frag
#extension GL_ARB_bindless_texture : require

layout (location=0) in vec3 vPos; // In M Space
layout (location=1) in vec3 vNom; // In MV Space
layout (location=2) in vec2 vTex;
layout (location=3) in flat uint vInstance;

layout (location=0)	out vec4 fFragColor; // location 0 is default GL_BACK_LEFT color buffer
layout (location=1) out uint fGID;
layout (location=2) out vec3 fPositionData;
layout (location=3) out vec3 fNormalData;
layout (location=4) out vec4 fMetalRoughData;

struct InstanceData
{
	mat4 mdlMtx;
	vec4 color;
	float roughness;
	float metallic;
	unsigned int entityID;
	unsigned int textureID; 
};

layout(binding=0, std430) readonly buffer ssbo3
{
	sampler2D textures[];
};

layout(binding=2, std430) readonly buffer ssbo1
{
	InstanceData iDat[];
};

void main(void){
	fPositionData = vPos;
	fNormalData = normalize(vNom);
	fFragColor = texture(textures[iDat[vInstance].textureID], vTex) * vec4(iDat[vInstance].color.rgb, 0.5);
	fGID = iDat[vInstance].entityID;
	fMetalRoughData.xy = vec2(iDat[vInstance].roughness, iDat[vInstance].metallic);
}
