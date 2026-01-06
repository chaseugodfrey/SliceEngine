#version 460 core
//upSample_Frag
layout (location=0) in vec2 vTexCoord;

layout (location=0)	out vec3	fFragColor; // location 0 is default GL_BACK_LEFT color buffer

layout (binding = 0) uniform sampler2D uTex; // Texture
uniform float uFilterRadius;
const float cAspectRatio  = 1.7777777777;

void main(void){
	float x = uFilterRadius;
	float y = uFilterRadius * cAspectRatio;

	vec3 a = texture(uTex, vec2(vTexCoord.x - x, vTexCoord.y + y)).rgb;
	vec3 b = texture(uTex, vec2(vTexCoord.x,	 vTexCoord.y + y)).rgb;
	vec3 c = texture(uTex, vec2(vTexCoord.x + x, vTexCoord.y + y)).rgb;
	
	vec3 d = texture(uTex, vec2(vTexCoord.x - x, vTexCoord.y)).rgb;
	vec3 e = texture(uTex, vec2(vTexCoord.x,	 vTexCoord.y)).rgb;
	vec3 f = texture(uTex, vec2(vTexCoord.x + x, vTexCoord.y)).rgb;
	
	vec3 g = texture(uTex, vec2(vTexCoord.x - x, vTexCoord.y - y)).rgb;
	vec3 h = texture(uTex, vec2(vTexCoord.x,	 vTexCoord.y - y)).rgb;
	vec3 i = texture(uTex, vec2(vTexCoord.x + x, vTexCoord.y - y)).rgb;

	fFragColor = e * 4.0;
	fFragColor += (b+d+f+h)*2.0;
	fFragColor += (a+c+g+i);
	fFragColor *= 1.0 / 16.0;
}
