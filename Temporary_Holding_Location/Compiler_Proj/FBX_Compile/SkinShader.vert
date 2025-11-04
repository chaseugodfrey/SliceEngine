#version 450 core

layout (location=0) in vec3 v_position;
layout (location=1) in vec3 v_normal;
layout (location=2) in vec2 v_texCoord;
layout (location=3) in ivec4 v_boneid;
layout (location=4) in vec4 v_weights;


//layout (location=0) out vec3 i_color;
layout (location=0) out vec3 i_normal;
layout (location=1) out vec2 i_texCoord;
layout (location=2) out vec3 i_position;

uniform vec2 offset;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 finalBonesMatrices[MAX_BONES];

void main(){
    vec4 final_pos = vec4(v_position, 1.0f);
    mat4 bone_tform = mat4(0.0f);
    bool is_bone_animated = false;
    for(int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
        int bone_id = v_boneid[i];

        if(bone_id == -1) {
            continue;
        }
        is_bone_animated = true;
        bone_tform += finalBonesMatrices[bone_id] * v_weights[i];
    }
    
    vec4 local_norm = vec4(v_normal, 1.0f);
	mat4 MV = V * M;

    if(is_bone_animated) {
        final_pos = bone_tform * vec4(v_position, 1.0f);
        //final_pos = finalBonesMatrices[1] * vec4(v_position, 1.0f);
        local_norm = bone_tform * local_norm;
        MV = V;
    }

    mat3 N = mat3(vec3(MV[0]), vec3(MV[1]), vec3(MV[2])); 
    i_normal = normalize(N * local_norm.xyz);

    vec4 VertexPositionInView = MV * vec4(final_pos);
    i_position = VertexPositionInView.xyz;

    gl_Position = P * VertexPositionInView; 

	i_texCoord	= v_texCoord;
}