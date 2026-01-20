#version 460 core

layout (location=0) in vec3 aVertexPosition;
layout (location=2) in vec2 aTextureCoord;

layout (location=0) out vec2 vTextureCoord;

uniform mat4 canvas_to_ndc;		//from canvas

struct Instance_Data{
	mat4 M;
	vec4 atlas_uv;
};

layout(std430, binding = 3) readonly buffer data {
	Instance_Data[] instances;
};

void main(void){
	Instance_Data instance = instances[gl_InstanceID];
	
	vec4 uv_map = instance.atlas_uv;
	vTextureCoord.x = mix(uv_map.x, uv_map.y, aTextureCoord.x);
	vTextureCoord.y = mix(uv_map.z, uv_map.w, aTextureCoord.y);

	//vTextureCoord = aTextureCoord;
	
	gl_Position	=  canvas_to_ndc * instance.M * vec4(aVertexPosition, 1.0);
	//gl_Position = vec4(aVertexPosition, 1.0);
}