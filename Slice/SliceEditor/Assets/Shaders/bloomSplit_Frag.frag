#version 460 core
//bloomSplit_Frag
layout (location=0) in vec2 vTexCoord;

layout (location=0)	out vec3	fFragColor; // location 0 is default GL_BACK_LEFT color buffer

layout (binding = 0) uniform sampler2D uTex; // Texture
layout (binding = 1) uniform sampler2D uEmission;
const vec3 cThreshold = vec3(0.2126, 0.7152, 0.0722);
uniform float uLimit = 1.0;

void main(void){
	vec3 col = texture(uTex, vTexCoord).rgb; // Texure after lighting + emission
	fFragColor = texture(uEmission, vTexCoord).rgb; // just the emission

	float brightness = dot(col, cThreshold);
	if(brightness > uLimit)
	{
		fFragColor = col;
	}
}
