#version 460 core

layout (location=0) in vec3 aVertexPosition;
layout (location=1)	in vec3	aNom;
layout (location=2)	in vec2	aTex;

layout (location=3) in ivec4 aBoneid;	//for skin mesh, only accessed if uniform bool is true
layout (location=4) in vec4 aWeights;
layout (location=5) in vec3 aTan;

layout (location=0) out vec3 vPos; // World Space
layout (location=1) out vec3 vNom;
layout (location=2) out vec2 vTex;
layout (location=3) out flat uint vInstance;
layout (location=4) out mat3 vTBN;

struct BasicIDat
{
	mat4 mdlMtx;
	uint entityID;
	uint textureID; 
	uint tex2ID;
	uint col;
};

layout(binding=1, std430) readonly buffer ssbo1
{
	BasicIDat iDat[];
};

uniform uint aGID;
uniform mat4 V; // View transform matrix
uniform mat4 P; // Perspective transform matrix

//mesh skinning
const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

uniform bool skinned;
uniform mat4 inverse_root;
uniform mat4 final_bones_matrices[MAX_BONES];

void main(void){
	
    bool is_bone_animated = false;
    mat4 bone_tform = mat4(0.0f);

	if(skinned) {
		for(int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
			int bone_id = aBoneid[i];

			if(bone_id == -1) {
				continue;
			}
			is_bone_animated = true;
			bone_tform += final_bones_matrices[bone_id] * aWeights[i];
		}
	}

	mat4 model_to_world = iDat[gl_InstanceID].mdlMtx;

	if(is_bone_animated) {
		model_to_world = model_to_world * inverse_root * bone_tform;
	}

	mat3 N = transpose(inverse(mat3(model_to_world)));
	vec4 posInWorld = model_to_world * vec4(aVertexPosition, 1.0);

	vPos = posInWorld.xyz;

	vNom = normalize(N * aNom);
	vec3 normal = normalize(model_to_world * vec4(aNom, 0.0f)).xyz;
	vec3 tangent = normalize(model_to_world * vec4(aTan, 0.0f)).xyz;
	tangent = normalize(tangent - dot(tangent, normal) * normal);
	vec3 bitangent = cross(normal, tangent);
	vTBN = mat3(tangent, bitangent, normal);

	vTex = aTex;
	vInstance = gl_InstanceID;
	gl_Position	= P * V * posInWorld;
}