#version 460 core
layout (location=0)	in vec2				vTexCoord;


layout (binding = 0) uniform sampler2D tex2d;
uniform float alphaThresh;	//from sprite renderer
uniform unsigned int entity;//from sprite renderer
uniform bool raycast_target;//from sprite renderer


layout (location=1)	out unsigned int	eid; // location 1 is id buffer

void main(void){

	vec4 texture_color = texture(tex2d, vTexCoord);
	
	bool set_raycast = raycast_target && (texture_color.a >= alphaThresh);

	if(!set_raycast) {
		discard;
	}


	eid = entity;

}
