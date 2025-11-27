#version 460 core
layout (location=0)	in vec2				vTexCoord;

layout (location=0)	out vec4			fFragColor; // location 0 is color buffer
layout (location=1)	out unsigned int	eid; // location 1 is id buffer

uniform bool raycast;		//from canvas

uniform vec4 rgba;			//from sprite renderer
uniform float alphaThresh;	//from sprite renderer
uniform unsigned int entity;//from sprite renderer
uniform bool raycast_target;//from sprite renderer

layout (binding = 0) uniform sampler2D tex2d;
layout (binding = 1) uniform usampler2D eid_tex;	//both input and output xdd bless opengl 4.5, ref:
													//https://www.reddit.com/r/opengl/comments/zoe1wj/the_same_texture_for_input_and_output/

void main(void){

	vec4 texture_color = texture(tex2d, vTexCoord);
	fFragColor = texture_color * rgba;
	
	bool set_raycast = raycast && raycast_target && (texture_color.a >= alphaThresh);
	
	ivec2 xy = ivec2(gl_FragCoord.xy);
	unsigned int dest_eid = texelFetch(eid_tex, xy, 0).r;	//get the current eid value
	if(set_raycast) {
		dest_eid = entity;
	}

	eid = dest_eid;

}
