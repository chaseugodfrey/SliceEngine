#version 460 core
layout (location=0)	in vec2				vTexCoord;


layout (binding = 0) uniform sampler2D tex2d;
uniform vec4 rgba;			//from sprite renderer


layout (location=0)	out vec4			fFragColor; // location 0 is color buffer

void main(void){

	vec4 texture_color = texture(tex2d, vTexCoord);
	fFragColor = texture_color * rgba;
}
