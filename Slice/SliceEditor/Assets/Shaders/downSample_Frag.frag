#version 460 core
//downSample_Frag
layout (location=0) in vec2 vTexCoord;

layout (location=0)	out vec3	fFragColor; // location 0 is default GL_BACK_LEFT color buffer

layout (binding = 0) uniform sampler2D uTex; // Texture
uniform vec2 uTexelSize;	// Resolution

void main(void){
	float x = uTexelSize.x;
	float y = uTexelSize.y;

	vec3 a = texture(uTex, vec2(vTexCoord.x - 2*x, vTexCoord.y + 2*y)).rgb;
	vec3 b = texture(uTex, vec2(vTexCoord.x,	   vTexCoord.y + 2*y)).rgb;
	vec3 c = texture(uTex, vec2(vTexCoord.x + 2*x, vTexCoord.y + 2*y)).rgb;
	
	vec3 d = texture(uTex, vec2(vTexCoord.x - 2*x, vTexCoord.y)).rgb;
	vec3 e = texture(uTex, vec2(vTexCoord.x,	   vTexCoord.y)).rgb;
	vec3 f = texture(uTex, vec2(vTexCoord.x + 2*x, vTexCoord.y)).rgb;
	
	vec3 g = texture(uTex, vec2(vTexCoord.x - 2*x, vTexCoord.y - 2*y)).rgb;
	vec3 h = texture(uTex, vec2(vTexCoord.x,	   vTexCoord.y - 2*y)).rgb;
	vec3 i = texture(uTex, vec2(vTexCoord.x + 2*x, vTexCoord.y - 2*y)).rgb;
	
	vec3 j = texture(uTex, vec2(vTexCoord.x - x, vTexCoord.y + y)).rgb;
	vec3 k = texture(uTex, vec2(vTexCoord.x + x, vTexCoord.y + y)).rgb;
	vec3 l = texture(uTex, vec2(vTexCoord.x - x, vTexCoord.y - y)).rgb;
	vec3 m = texture(uTex, vec2(vTexCoord.x + x, vTexCoord.y - y)).rgb;

	fFragColor = e * 0.125;
	fFragColor += (a+c+g+i) * 0.03125;
	fFragColor += (b+d+f+h) * 0.0625;
	fFragColor += (j+k+l+m) * 0.125;
	fFragColor = max(fFragColor, 0.0001);
}
