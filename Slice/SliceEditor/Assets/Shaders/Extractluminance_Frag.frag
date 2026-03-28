#version 460 core
//Extractluminance_Frag
layout (location=0) in vec2 vTexCoord;

layout (location=0)	out vec4 fFragColor;

layout (binding = 0) uniform sampler2D 	uTex;

const vec3 lumWeight = vec3(0.2126, 0.7152, 0.0722);
const float minLum = 0.00001;

/***************************************************
* Out: fFragColor
*****************************************************/
void main(void){
	vec4 col = texture(uTex, vTexCoord);
	float colLum = minLum;
	if(!any(isnan(col)))
		colLum = max(dot(col.rgb, lumWeight), minLum);
	float logLum = log(colLum);

	fFragColor = vec4(logLum, 0.0, 0.0, 1.0);
}
