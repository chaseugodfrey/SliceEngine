#version 460 core
//skyboxLight_Frag
layout (location=0) in vec2 vTexCoord;

layout (location=0)	out vec4 fFragColor;

layout (binding = 0) uniform sampler2D 	uTex;
layout (binding = 1) uniform sampler2D 	uNomTex;
layout (binding = 2) uniform samplerCube uCubemapTex;

void main(void){
	ivec2 p = ivec2(gl_FragCoord.xy);
	vec3 nom = texelFetch(uNomTex, p, 0).xyz;
	vec3 dif = texelFetch(uTex, p, 0).rgb;
	vec3 ambient = texture(uCubemapTex, nom).rgb;

	if(any(notEqual(nom, vec3(0.0f))))
	{
		fFragColor = vec4(dif * ambient, 1.0);
	}
	else
	{
		fFragColor = vec4(dif, 1.0);
	}
}
