#version 460 core
//bloomSplit_Frag
layout (location=0) in vec2 vTexCoord;

layout (location=0)	out vec3	fFragColor; // location 0 is default GL_BACK_LEFT color buffer

layout (binding = 0) uniform sampler2D uTex; // Texture
const vec3 cThreshold = vec3(0.2126, 0.7152, 0.0722);
uniform float uLimit = 1.0;

void main(void){
	vec3 col = texture(uTex, vTexCoord).rgb;

	float brightness = dot(col, cThreshold);
	if(brightness > uLimit)
	{
		fFragColor = col;
	}
	else
	{
		fFragColor = vec3(0.0,0.0,0.0);
	}

}
