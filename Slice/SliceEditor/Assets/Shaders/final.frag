#version 460 core

layout (location=0)	out vec4 fFragColor; // location 0 is default GL_BACK_LEFT color buffer

layout (binding = 0) uniform sampler2D 	uTex; // Already undergone the addition of all objects

uniform float uExposure = 1.0;

const vec3 gamma = vec3(0.45454545454);

// if doing instance rendering, save bindings 12~15 // could lower to 13~15

/***************************************************
* Out: fFragColor
*****************************************************/
void main(void){
	ivec2 p = ivec2(gl_FragCoord.xy);
	vec3 hdrCol = texelFetch(uTex, p, 0).rgb;

	// Tone Mapping
	hdrCol = vec3(1.0) - exp(-hdrCol * uExposure);
	// Gamma Correction
	hdrCol = pow(hdrCol, gamma); // Gamma Correction
	fFragColor = vec4(hdrCol, 1.0);
}
