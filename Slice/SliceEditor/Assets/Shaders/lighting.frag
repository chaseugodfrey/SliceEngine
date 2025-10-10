#version 460 core

struct Light{
	vec3 position;
	vec3 color;
	int type;
};

layout (location=0)	out vec4 fFragColor; // location 0 is default GL_BACK_LEFT color buffer

const float PI = 3.14159265358979323846;
const float EPSILON = 0.000001;
// -TODO- Temporary material values
const float tR = 0.3f;
const float tM = 0.0f;
const int isDirectional = 1;
const int isPoint 		= 2;
const int isSpot 		= 3;

uniform mat4 uLightMtx; // Shadow Transform Matrix
uniform Light uLight;
uniform mat4 V;

layout (binding = 0) uniform sampler2D 	uTex;
layout (binding = 1) uniform sampler2D 	uPosTex;
layout (binding = 2) uniform sampler2D 	uNomTex;
layout (binding = 3) uniform sampler2D 	uShadowTex;
// if doing instance rendering, save bindings 12~15 // could lower to 13~15

vec3 microfacetModel(vec3 v, vec3 n, vec3 lightCol, vec3 l, vec3 dif);

/***************************************************
* Out: fFragColor (Addictive)
*****************************************************/
void main(void){
	ivec2 p = ivec2(gl_FragCoord.xy);
	vec3 pos = texelFetch(uPosTex, p, 0).xyz;
	vec3 nom = texelFetch(uNomTex, p, 0).xyz;
	vec4 dif = texelFetch(uTex, p, 0);

	vec4 vLightPos = uLightMtx * vec4(pos, 1.0f);
	vec3 projCoords = vLightPos.xyz / vLightPos.w;
	projCoords = projCoords * 0.5f + 0.5f;
	float shadowDepth = texture(uShadowTex, projCoords.xy, 0).r;
	float shadow = projCoords.z > shadowDepth ? 1.0 : 0.0;

	vec3 viewPos = (V * vec4(pos, 1.0f)).xyz;
	vec3 v = normalize(-viewPos);

	if(any(notEqual(nom, vec3(0.0f))) && abs(dif.a) > EPSILON)
	{
		if(uLight.type == isDirectional)
		{
			vec3 ambient = dif.rgb * 0.01; // if blocked by shadow

			vec3 l = uLight.position;
			ambient += (1.0 - shadow) * microfacetModel(v, nom, uLight.color, l, dif.rgb);
			fFragColor = vec4(ambient, 1.0f);
		}
		else if(uLight.type == isPoint)
		{
			vec3 lightPosInView = (V * vec4(uLight.position, 1.0f)).xyz;
			vec3 l = uLight.position - viewPos; // light vector
			float dist = length(l);
			vec3 lightCol = uLight.color * 100 / (dist * dist); // Insensity is normalized, so scale up by 100?

			fFragColor = vec4(((1.0 - shadow) * microfacetModel(v, nom, lightCol, l, dif.rgb)), 1.0f);
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
	float d = (nDotH * nDotH) * (alpha2 - 1.0f) + 1.0f;
	return alpha2 / (PI * d * d);
}

vec3 SchlickFresnel(float lDotH, vec3 dif, float metal)
{
	vec3 f0 = vec3(0.04f); // -TODO- Dielectrics
	if(metal == 1.0f)
		f0 = dif;
	return f0 + (1.0f - f0) * pow(1.0f - lDotH, 5);
}

float GeomSmith(float nDotL, float rough)
{
	float k = (rough + 1.0f) * (rough + 1.0f) / 8.0f;
	float d = nDotL * (1.0f - k) + k;
	return 1.0f / d;
}

vec3 microfacetModel(vec3 v, vec3 n, vec3 lightCol, vec3 l, vec3 dif)
{
	l = normalize(l);

	vec3 h = normalize(v + l);
	float nDotH = dot(n, h);
	float lDotH = dot(l, h);
	float nDotL = max(dot(n, l), 0.0f);
	float nDotV = dot(n, v);
	
	vec3 specBrdf = 0.25f * GgxDistribution(nDotH, tR) * SchlickFresnel(lDotH, dif, tM) *  GeomSmith(nDotL, tR) * GeomSmith(nDotV, tR);

	return (dif + PI * specBrdf) * lightCol * nDotL;
}