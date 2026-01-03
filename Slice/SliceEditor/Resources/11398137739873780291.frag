#version 460 core

layout (location=0) in vec4 gFragPos;

layout (location=0)	out vec4 fFragColor;

layout (binding = 0) uniform samplerCube uCubemapTex;

const float PI			= 3.14159265359;
const float PI_05		= 1.5707963268;
const float PI_2		= 6.28318530718;
const float sampleDelta = 0.025;

void main(void){
	vec3 nom = normalize(gFragPos.xyz);
	vec3 irradiance = vec3(0.0);

	vec3 up	= vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, nom));
	up			= normalize(cross(nom, right));
	
	int nrSamples = 0;
	for(float phi = 0.0; phi < PI_2; phi += sampleDelta)
	{
		for(float theta = 0.0; theta < PI_05; theta += sampleDelta)
		{
			float cTheta = cos(theta);
			float sTheta = sin(theta);

			vec3 tanSample = vec3(sTheta * cos(phi), sTheta * sin(phi), cTheta);
			vec3 vecSample = tanSample.x * right + tanSample.y * up + tanSample.z * nom;

			irradiance += texture(uCubemapTex, vecSample).rgb * cTheta * sTheta;
			nrSamples++;
		}
	}

	irradiance = PI * irradiance * (1.0 / float(nrSamples));

	fFragColor = vec4(irradiance, 1.0);
}
