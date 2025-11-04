#version 460 core

layout (location=0)	out vec4 fFragColor; // location 0 is default GL_BACK_LEFT color buffer

layout (binding = 0) uniform sampler2D 	uTex; // Already undergone the addition of all objects
layout (binding = 1) uniform sampler2D 	uBloom; // The Bloom Texture

uniform bool uIsBloom;
uniform float uExposure = 1.0;
uniform float uBloomStrength = 0.04;

const vec3 gamma = vec3(0.45454545454);

// if doing instance rendering, save bindings 12~15 // could lower to 13~15

/***************************************************
* Out: fFragColor
*****************************************************/
void main(void){
	vec3 result = vec3(0.0);

	ivec2 p = ivec2(gl_FragCoord.xy);
	vec3 hdrCol = texelFetch(uTex, p, 0).rgb;
	if(uIsBloom)
	{
		vec3 bloomCol = texelFetch(uBloom, p, 0).rgb;
		result = mix(hdrCol, bloomCol, uBloomStrength);
	}
	else
	{
		result = hdrCol;
	}
	// Tone Mapping
	result = vec3(1.0) - exp(-result * uExposure);
	// Gamma Correction
	result = pow(result, gamma); // Gamma Correction
	fFragColor = vec4(result, 1.0);
}
