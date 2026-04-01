#version 460 core
//skyboxLight_Frag
layout (location=0) in vec2 vTexCoord;

layout (location=0)	out vec4 fFragColor;

layout (binding = 0) uniform sampler2D 	uTex;
layout (binding = 1) uniform sampler2D 	uNomTex;
layout (binding = 2) uniform samplerCube uCubemapTex;
layout (binding = 3) uniform sampler2D	uEmission;
layout (binding = 4) uniform sampler2D 	uRoughMetalLightTex;

uniform float skyboxLightingPower = 1.0f;
uniform bool willBloom = false;

void main(void){
	ivec2 p = ivec2(gl_FragCoord.xy);
	vec3 nom = texelFetch(uNomTex, p, 0).xyz;
	vec3 dif = texelFetch(uTex, p, 0).rgb;
	vec3 ambient = texture(uCubemapTex, nom).rgb;
	vec3 emission = vec3(0.0f);

	float toUseLight =  texelFetch(uRoughMetalLightTex, p, 0).z;

	//if(!willBloom)
		emission = texelFetch(uEmission, p, 0).rgb;

	fFragColor = vec4(dif, 1.0);
	
	if(any(notEqual(nom, vec3(0.0f))) && toUseLight < 0.5f)
	{
		fFragColor = vec4(dif * ambient * skyboxLightingPower, 1.0);
	}
	fFragColor += vec4(emission, 0.0);
}
