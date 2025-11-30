#version 460 core

layout (location=0) in vec2 vTexCoord;

layout (location=0)	out vec4 fFragColor;

layout (binding = 0) uniform sampler2D 	uTex; // Already undergone the addition of all objects
layout (binding = 1) uniform sampler2D 	uBloom; // The Bloom Texture

uniform float uBloomStrength;

void main(void){
	vec3 hdrCol = texture(uTex, vTexCoord).rgb;
	vec3 bloomCol = texture(uBloom, vTexCoord).rgb * uBloomStrength;

	fFragColor = vec4((hdrCol + bloomCol), 1.0);
}
