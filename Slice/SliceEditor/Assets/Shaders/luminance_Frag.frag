#version 460 core
//luminance_Frag
layout (location=0) in vec2 vTexCoord;

layout (location=0)	out vec4 fFragColor; // location 0 is default GL_BACK_LEFT color buffer

layout (binding = 0) uniform sampler2D 	uTex; // Already undergone the addition of all objects
layout (binding = 1) uniform sampler2D  uLastLumi;

uniform float uLearningRate = 10.0f;

/***************************************************
* Out: fFragColor
*****************************************************/
void main(void){
	vec4 addition = vec4(0.0);
	for(int x = 0; x < 3; ++x)
	{
		for(int y = 0; y < 2; ++y)
		{
			vec4 a = textureLod(uTex, vec2(float(x),float(y)), 9.0);
			if(!any(isnan(a)))
				addition += a;
		}
	}
	addition /= 6.0;
	
	float avgLum = dot(addition.rgb, vec3(0.2126, 0.7152, 0.0722));

	float lastLum = texture(uLastLumi, vec2(0,0)).r;

	float lr = uLearningRate / 1000.0;

	float finalVal = lastLum * (1.0 - lr) + avgLum * lr;
	fFragColor = vec4(finalVal, 0.0, 0.0, 1.0);
}
