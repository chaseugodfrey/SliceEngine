#version 460 core

layout (location=0) in vec2 vTexCoord;

layout (location=0) out vec4 fFrag;

layout (binding = 0) uniform usampler2D uTex;

uniform int uBlur = 1;
uniform int uWidth = 5;
uniform vec3 uCol;

void main(void){
	ivec2 pix = ivec2(gl_FragCoord.xy);
	float intensity = float(texelFetch(uTex, pix, 0).r);

	if(uBlur == 1)
	{
		float colMul = (intensity / float(uWidth * 2));

		fFrag = vec4(uCol, colMul);
	}
	else
	{
		if(intensity > 0.5)
			fFrag = vec4(uCol, 1.0);
	}
}