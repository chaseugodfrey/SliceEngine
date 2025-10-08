#version 460 core

struct Light{
	vec3 position;
	vec3 color;
};

layout (location=0) in vec3 vPos; // In MV Space
layout (location=1) in vec3 vNom; // In MV Space
layout (location=2) in vec2 vTexCoord;
layout (location=3) in flat uint vGID;

layout (location=0)	out vec4 fFragColor; // location 0 is default GL_BACK_LEFT color buffer
layout (location=1) out uint fGID;
layout (location=2) out vec3 fPositionData;
layout (location=3) out vec3 fNormalData;
layout (location=4) out vec3 fDiffuseColor;

const int cMaxNumLights = 32;
const float PI = 3.14159265358979323846;
const float EPSILON = 0.000001;
// -TODO- Temporary material values
const float tR = 0.3f;
const float tM = 0.0f;

uniform int uPass;
uniform int numLights;
uniform Light uLight[cMaxNumLights];
uniform mat4 V;

layout (binding = 0) uniform sampler2D 	uTex;
layout (binding = 1) uniform sampler2D 	uPosTex;
layout (binding = 2) uniform sampler2D 	uNomTex;
// if doing instance rendering, save bindings 12~15 // could lower to 13~15

vec3 BlingPhongDirectional(vec3 pos, vec3 nom, Light light, vec3 mat);
vec4 BRDFAll(vec3 pos, vec3 n, Light light, vec4 dif);

/***************************************************
* Out: fPositionData, fNormalData
*****************************************************/
void Pass0(){
	fPositionData = vPos;
	fNormalData = normalize(vNom);
	fDiffuseColor = vec3(texture(uTex, vTexCoord));
	fGID = vGID;
}

/***************************************************
* Out: fFragColor
*****************************************************/
void Pass1(){
	ivec2 p = ivec2(gl_FragCoord.xy);
	vec3 pos = texelFetch(uPosTex, p, 0).xyz;
	vec3 nom = texelFetch(uNomTex, p, 0).xyz;
	vec4 dif = texelFetch(uTex, p, 0);

	if(any(notEqual(nom, vec3(0.0f))))
	{
		fFragColor = BRDFAll(pos, nom, uLight[0], dif);
	}
	else // background
	{
		fFragColor = vec4(0.75294f, 1.0f, 0.93333f, 1.0f);
	}
	fFragColor.rgb = pow(fFragColor.rgb, vec3(1.0f/2.2f));
}

void main(void){
	if		(uPass == 0) Pass0();
	else if (uPass == 1) Pass1();
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

vec3 BRDF(vec3 v, vec3 n, vec3 lightCol, vec3 l, vec3 dif)
{
	float dist = length(l);
	l = normalize(l);
	lightCol *= 100 / (dist * dist); // Insensity is normalized, so scale up by 100?

	vec3 h = normalize(v + l);
	float nDotH = dot(n, h);
	float lDotH = dot(l, h);
	float nDotL = max(dot(n, l), 0.0f);
	float nDotV = dot(n, v);
	
	vec3 specBrdf = 0.25f * GgxDistribution(nDotH, tR) * SchlickFresnel(lDotH, dif, tM) *  GeomSmith(nDotL, tR) * GeomSmith(nDotV, tR);

	return (dif + PI * specBrdf) * lightCol * nDotL;
}

vec4 BRDFAll(vec3 pos, vec3 n, Light light, vec4 dif)
{
    if (abs(dif.a) < EPSILON)
		return vec4(0.0f);

	vec3 calcCol = vec3(0.0f);
	vec3 v = normalize(-pos);

	// BlingPhongDirectional(pos, n, uLight[0], dif.rgb);
	{
		vec3 l = (V * vec4(uLight[0].position, 0.0f)).xyz;
		l = normalize(l);
		vec3 h = normalize(v + l);
		float nDotH = dot(n, h);
		float lDotH = dot(l, h);
		float nDotL = max(dot(n, l), 0.0f);
		float nDotV = dot(n, v);
		
		vec3 specBrdf = 0.25f * GgxDistribution(nDotH, tR) * SchlickFresnel(lDotH, dif.rgb, tM) *  GeomSmith(nDotL, tR) * GeomSmith(nDotV, tR);
	
		calcCol = (dif.rgb + PI * specBrdf) * light.color * nDotL;
	}

	for(int i = 1; i < numLights; ++i)
	{
		vec3 lightPosInView = (V * vec4(uLight[i].position, 1.0f)).xyz;
		vec3 l = lightPosInView - pos;
		calcCol += BRDF(v, n, uLight[i].color, l, dif.rgb);
	}

	return vec4(calcCol, 1.0f);
}

const float cAmbientInensity = 0.05f;
const float cMatShinyness = 32.0f;

vec3 BlingPhongDirectional(vec3 pos, vec3 nom, Light light, vec3 mat)
{
	vec3 color = mat * light.color * cAmbientInensity;// ambient
	if(any(notEqual(light.position, vec3(0.0f))))
	{
		vec3 toLight = normalize((V * vec4(light.position, 0.0f)).xyz);
		float cosTheta = max(dot(toLight, nom), 0.0f);
		vec3 diffuse = mat * light.color * cosTheta;

		vec3 specular = vec3(0.0f);
		// Don't need calculate specular if angle is >90
		if(cosTheta > 0.0f)
		{
			pos = normalize(-pos);
			vec3 h = normalize(pos + toLight);
			float cosPhi = max(dot(h, nom), 0.0f);
			specular = mat * light.color * pow(cosPhi, cMatShinyness);
		}
		color += diffuse + specular;
	}
	return color;
}
