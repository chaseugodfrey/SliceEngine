#version 460 core
//luminance_Frag
layout (location=0) in vec2 vTexCoord;

layout (location=0)	out vec4 fFragColor; // location 0 is default GL_BACK_LEFT color buffer

layout (binding = 0) uniform sampler2D 	uTex; // Already undergone the addition of all objects
layout (binding = 1) uniform sampler2D  uLastLumi;

uniform float uLearningRate = 10.0f;
uniform float uMaxLum = 100.0;
uniform float uMinLum = 0.001;

const float minLum = 0.00001;
/***************************************************
* Out: fFragColor
*****************************************************/
void main(void){
	float logLumSum = 0.0;
	float numSamples = 0.0;

	for(int x = 0; x <= 6; ++x)
	{
		for(int y = 0; y <= 3; ++y)
		{
			float a = textureLod(uTex, vec2((float(x) + 0.5) / 7.0,(float(y) + 0.5) / 4.0), 8.0).r;

			float logLum = minLum;
			if(!isnan(a))
				logLum = a;

			logLumSum += logLum;
			numSamples += 1.0;
		}
	}
	logLumSum /= numSamples;


	float avgLum = min(max(exp(logLumSum), uMinLum),uMaxLum);
 
	float lastLum = texture(uLastLumi, vec2(0.5)).r;

	float lr = min(max(uLearningRate / 1000.0, 0.0), 1.0);

	float finalVal = lastLum * (1.0 - lr) + avgLum * lr;
	fFragColor = vec4(finalVal, 0.0, 0.0, 1.0);
}
