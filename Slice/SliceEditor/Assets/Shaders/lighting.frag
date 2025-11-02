#version 460 core

struct Light{
	vec3 position;
	vec3 direction;
	vec4 color; // rgb + intensity
	float hasShadow;
	int type;
};

layout (location=0)	out vec4 fFragColor; // location 0 is default GL_BACK_LEFT color buffer

const float PI = 3.14159265358979323846;
const float EPSILON = 0.000001;
// -TODO- Temporary material values
const float ambient = 0.01;
const int isDirectional = 0;
const int isPoint 		= 1;
const int isSpot 		= 2;

uniform mat4 uLightMtx; // Shadow Transform Matrix
uniform Light uLight;
uniform vec3 uCamPos;
uniform float uRoughness;
uniform float uMetallic;

layout (binding = 0) uniform sampler2D 	uTex;
layout (binding = 1) uniform sampler2D 	uPosTex;
layout (binding = 2) uniform sampler2D 	uNomTex;
layout (binding = 3) uniform sampler2D 	uShadowTex;			// Only for shadow mapping (spot / directional light)
layout (binding = 4) uniform samplerCube 	uShadowCubeMap; // Only for shadow mapping (point light)
// if doing instance rendering, save bindings 12~15 // could lower to 13~15

float getShadowMulti(vec3 n, vec3 l, vec3 projCoords);
float getShadowCubeMulti(vec3 n, vec3 l, float viewDist, float dist);
vec3 microfacetModel(vec3 v, vec3 n, vec3 lightCol, vec3 l, vec3 dif);

/***************************************************
* Out: fFragColor (Addictive)
*****************************************************/
void main(void){
	ivec2 p = ivec2(gl_FragCoord.xy);
	vec3 wPos = texelFetch(uPosTex, p, 0).xyz;// In World Space
	vec3 nom = texelFetch(uNomTex, p, 0).xyz;
	vec4 dif = texelFetch(uTex, p, 0);

	if(any(notEqual(nom, vec3(0.0f))) && abs(dif.a) > EPSILON)
	{
		nom = normalize(nom);
		vec4 vLightPos = uLightMtx * vec4(wPos, 1.0f);
		vec3 projCoords = vLightPos.xyz / vLightPos.w;
		projCoords = projCoords * 0.5f + 0.5f;

		vec3 v = normalize(uCamPos - wPos);

		if(uLight.type == isDirectional)
		{
			vec3 ambient = dif.rgb * ambient; // if blocked by shadow

			vec3 l = normalize(-uLight.direction);// Surface to Light
			float shadow = uLight.hasShadow * getShadowMulti(nom, l, projCoords);
			ambient += (1.0 - shadow) * microfacetModel(v, nom, uLight.color.rgb * uLight.color.a, l, dif.rgb);
			fFragColor = vec4(ambient, 1.0f);
		}
		else if(uLight.type == isPoint)
		{
			vec3 l = uLight.position - wPos; // Surface to Light
			float dist = length(l);
			vec4 lightCol = uLight.color;
			lightCol.a /= (dist * dist); // Insensity is normalized, so scale up by 100?

			float shadow = uLight.hasShadow * getShadowCubeMulti(nom, l, length(uCamPos - wPos), dist);
			l = l / dist;
			fFragColor = vec4(((1.0 - shadow) * microfacetModel(v, nom, lightCol.rgb * lightCol.a, l, dif.rgb)), 1.0f);
		}
	}
	else if(!any(notEqual(nom, vec3(0.0f))) && uLight.type == isDirectional)
	{
		fFragColor = vec4(0.75294f, 1.0f, 0.93333f, 1.0f);
	}
}


float GgxDistribution(float nDotH, float rough)
{
	float alpha2 = rough * rough * rough * rough;
	float d = nDotH * nDotH * (alpha2 - 1.0) + 1.0;
	return alpha2 / (PI * d * d);
}

vec3 SchlickFresnel(float lDotH, vec3 dif, float metal)
{
	vec3 f0 = vec3(0.04); // -TODO- Dielectrics
	//if(metal == 1.0)
	//	f0 = dif;
	//return f0 + (1.0 - f0) * pow(1.0 - lDotH, 5.0);

	f0 = mix(f0, dif, metal);
	return f0 + (1.0 - f0) * pow(clamp(1.0 - lDotH, 0.0, 1.0), 5.0);
}

float GeomSmith(float nDotL, float rough)
{
	float k = (rough + 1.0) * (rough + 1.0) / 8.0;
	float d = nDotL * (1.0 - k) + k;
	return nDotL / d;
}

vec3 microfacetModel(vec3 v, vec3 n, vec3 lightCol, vec3 l, vec3 dif)
{
	vec3 h = normalize(v + l);
	float nDotH = clamp(dot(n, h), 0.0, 1.0);
	//float lDotH = clamp(dot(l, h), 0.0, 1.0);
	float vDotH = clamp(dot(v, h), 0.0, 1.0);
	float nDotL = clamp(dot(n, l), 0.0, 1.0);
	float nDotV = abs(dot(n, v)) + 1e-5;
	
	//vec3 specBrdf = 0.25f * GgxDistribution(nDotH, uRoughness) * SchlickFresnel(lDotH, dif, uMetallic) *  GeomSmith(nDotL, uRoughness) * GeomSmith(nDotV, uRoughness);
	//return (dif + PI * specBrdf) * lightCol * nDotL;

	vec3 F = SchlickFresnel(vDotH, dif, uMetallic);
	vec3 kD = 1.0 - F;
	vec3 specBRDF_nom = GgxDistribution(nDotH, uRoughness) *
					F *
					GeomSmith(nDotL, uRoughness) *
					GeomSmith(nDotV, uRoughness);
	float specBRDF_denom = 4.0 * nDotV * nDotL + 1e-5;
	vec3 specBPDF = specBRDF_nom / specBRDF_denom;
	vec3 diffuseBRDF = kD * dif / PI;
	return (diffuseBRDF + specBPDF) * lightCol * nDotL;
}

float getShadowMulti(vec3 n, vec3 l, vec3 projCoords)
{
	float bias = max(0.005 * (1.0 - dot(n, l)), 0.0005);
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(uShadowTex, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(uShadowTex, projCoords.xy + vec2(x,y) * texelSize).r;
			shadow += projCoords.z - bias > pcfDepth ? 1.0 : 0.0;
		}
	}

	return shadow / 9.0;
}


// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float getShadowCubeMulti(vec3 n, vec3 l, float viewDist, float dist)
{
	vec3 fragToLight = -l;

	float bias = max(0.005 * (1.0 - dot(n, l)), 0.0005);
	float diskRadius = (1.0 + (viewDist / 20.0)) / 25.0;
	
	int samples = 20;
	float shadow = 0.0;
	for(int i = 0; i < samples; ++i)
	{
		float closestDepth = texture(uShadowCubeMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
		closestDepth *= 20.0;
		if(dist - bias > closestDepth)
			shadow += 1.0;
	}
	return shadow /= float(samples);
}