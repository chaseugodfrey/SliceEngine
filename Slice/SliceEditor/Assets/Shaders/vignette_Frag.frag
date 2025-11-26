#version 460 core

layout (location=0) in vec2 vTexCoord;

layout (location=0)	out vec4 fFragColor;

layout (binding = 0) uniform sampler2D 	uTex; // Already undergone the addition of all objects

uniform vec2 uVignetteCenter;
uniform float uVignetteIntensity;
uniform float uVignetteSmoothness;

void main(void){
	vec3 hdrCol = texture(uTex, vTexCoord).rgb;

	float dist = length(vTexCoord - uVignetteCenter);
	float outerRadius = 0.9 - uVignetteIntensity / 2.0;
	float innerRadius = (1.0 - uVignetteSmoothness) * outerRadius;
	float vignetteFactor = 1.0 - smoothstep(innerRadius, outerRadius, dist);

	fFragColor = vec4(hdrCol * vignetteFactor, 1.0);
}
