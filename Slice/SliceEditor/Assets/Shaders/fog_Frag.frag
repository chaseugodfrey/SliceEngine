#version 460 core
//fog_Frag
layout (location=0) in vec2 vTexCoord;

layout (location=0)	out vec4 fFragColor;

layout (binding = 0) uniform sampler2D 	uTex; // Already undergone the addition of all objects
layout (binding = 1) uniform sampler2D 	uPosTex;
layout (binding = 2) uniform sampler2D  uNomTex;

uniform vec3 uFogColor;
uniform float uFogIntensity;

void main(void){
	vec3 hdrCol = texture(uTex, vTexCoord).rgb;
	vec3 wPos = texture(uPosTex, vTexCoord).xyz; // In World Space
	vec3 nom = texture(uNomTex, vTexCoord).xyz; // has nom
	float dist = length(wPos);
	float fogFactor = 0.0;

	if(any(notEqual(nom, vec3(0.0f))))
	{
		fogFactor = 1.0 - exp(-dist * uFogIntensity);
	    fogFactor = clamp(fogFactor, 0.0, 1.0);
	}

	fFragColor = vec4(mix(hdrCol, uFogColor, fogFactor), 1.0);
}
