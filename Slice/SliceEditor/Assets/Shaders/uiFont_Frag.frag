#version 460 core
layout (location=0)	in vec2				vTexCoord;

layout (binding = 0) uniform sampler2D tex2d;
uniform vec4 rgba;			//from sprite renderer
uniform float relative_scale = 100.0;

layout (location=0)	out vec4			fFragColor; // location 0 is color buffer


const float onEdge = 128.0/256.0;
const float pixelDist = 64.0/256.0;

float unlerp(float min, float max, float v) {
	return (v - min) / (max - min);
}

void main(void){
	float sdf = texture(tex2d, vTexCoord).r;

	float sdf_dist = unlerp(onEdge, onEdge + pixelDist, sdf);
	float pixel_dist = sdf_dist * relative_scale;
	
	float value = unlerp(-0.5,0.5,pixel_dist);

	//value = smoothstep(1.0 - thickness, thickness, value);

	fFragColor = rgba;
	fFragColor.a *= clamp(value, 0, 1);

	if(fFragColor.a <= 0.0) {
		discard;
	}
	//not sure if make sense for fonts to have gamma, will check ltr
	//fFragColor.rgb = pow(fFragColor.rgb, gamma); // Gamma Correction
}
