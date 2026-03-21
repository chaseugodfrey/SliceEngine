#version 460 core
//cloud_Frag
struct Light{
	vec3 position;
	float uFarPlane;
	vec3 direction;
	int type;
	vec4 color; // rgb + intensity
	int hasShadow;
	int shadowNum;
	int spotShadowNum;
	int padding;
};

layout (location=0) in vec3 vPos;
layout (location=1) in vec3 vNom;
layout (location=2) in float vRandNoise;

layout (location=0)	out vec4 fFragColor; // location 0 is default GL_BACK_LEFT color buffer

layout (std140, binding = 0) uniform lightSpaceBlock
{
	mat4 lightSpaceMtx[16];
};
layout (std140, binding = 1) uniform lights
{
	Light directionLight;
	Light uLight[150];
};

layout (binding = 2) uniform samplerCube uSkyboxTex;
layout (binding = 4) uniform sampler2DArray uShadowTex;
layout (binding = 5) uniform samplerCubeArray 	uShadowCubeMap;

const float PI = 3.14159265358979323846;
const float EPSILON = 0.000001;
const float ambient = 0.01;
const float biasModifier = 0.5f;
const int isDirectional = 0;
const int isPoint 		= 1;
const int isSpot 		= 2;
const int maxLights = 10;
const float roughness = 1.0f;
const float metallic = 0.0f;


uniform mat4 V;
uniform int numLights;
uniform float cascadePlaneDist[16];
uniform int cascadeCnt;

uniform float uCloudsCutoff;
uniform vec4 uCloudsColor = vec4(1.f, 1.f, 1.f, 0.25f);

float getShadowMulti(vec3 n, vec3 l, vec3 projCoords, int layer);
float getShadowCubeMulti(vec3 n, vec3 l, float viewDist, float dist, int lightIdx, int numDirLights);
vec3 microfacetModel(vec3 v, vec3 n, vec3 lightCol, vec3 l, vec3 dif, float rough, float metal);
vec3 GetRandDir(vec3 seed);

void main(void){
	if(vRandNoise < uCloudsCutoff)
		discard;
	
	vec4 dif = uCloudsColor;
	vec3 nom = vNom;
	if(any(notEqual(nom, vec3(0.0f))))
	{
		nom = normalize(nom);

    	vec3 skyAmbient = texture(uSkyboxTex, nom).rgb;
    	fFragColor = vec4(dif.rgb * skyAmbient, dif.a);

    	// Copies lighting_Frag code
		vec3 v = normalize(-vPos);

		int numDirectionalLight = 0;
		for(int lightCnt = 0; lightCnt < maxLights; ++lightCnt)
    	{
    		if(uLight[lightCnt].type == isDirectional)
    		{
    	        if(numDirectionalLight > 0)
    	            continue; // -TODO- Only support 1 directional light for now
    			vec4 fragViewSpace = V * vec4(vPos, 1.0f);
    			float depthVal = abs(fragViewSpace.z);
    			int layer = -1;
    			for(int i = 0; i < cascadeCnt; ++i)
    			{
    				if(depthVal <= cascadePlaneDist[i])
    				{
    					layer = i;
    					break;
    				}
    			}
    			if(layer == -1)
    			{
    				layer = cascadeCnt - 1;
    			}

    			vec4 vLightPos = lightSpaceMtx[layer] * vec4(vPos, 1.0f);
    			vec3 projCoords = vLightPos.xyz / vLightPos.w;
    			projCoords = projCoords * 0.5f + 0.5f;

    			vec3 finalLighting = dif.rgb * ambient; // if blocked by shadow

    			vec3 l = normalize(-uLight[lightCnt].direction);// Surface to Light

    			//float shadow = getShadowMulti(nom, l, projCoords, layer);
    			//finalLighting += (1.0 - shadow) * microfacetModel(v, nom, uLight[lightCnt].color.rgb * uLight[lightCnt].color.a, l, dif.rgb, roughness, metallic);
				finalLighting += microfacetModel(v, nom, uLight[lightCnt].color.rgb * uLight[lightCnt].color.a, l, dif.rgb, roughness, metallic);

    			fFragColor += vec4(finalLighting, 0.0f);
    	        ++numDirectionalLight;
    		}
    		else if(uLight[lightCnt].type == isPoint)
    		{
    			vec3 l = uLight[lightCnt].position - vPos; // Surface to Light
    			float dist = length(l);
    			vec4 lightCol = uLight[lightCnt].color;
    			lightCol.a /= (dist * dist); // Intensity is normalized, so scale up by 100?

    			//float shadow = getShadowCubeMulti(nom, l, length(vPos), dist, lightCnt, numDirectionalLight);
    			l = l / dist;
    			//fFragColor += vec4(((1.0 - shadow) * microfacetModel(v, nom, lightCol.rgb * lightCol.a, l, dif.rgb, roughness, metallic)), 0.0f);
    			fFragColor += vec4(microfacetModel(v, nom, lightCol.rgb * lightCol.a, l, dif.rgb, roughness, metallic), 0.0f);
    		}
    	}
	}
	else
	{
	    fFragColor = dif;
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

	f0 = mix(f0, dif, metal);
	return f0 + (1.0 - f0) * pow(clamp(1.0 - lDotH, 0.0, 1.0), 5.0);
}

float GeomSmith(float nDotL, float rough)
{
	float k = (rough + 1.0) * (rough + 1.0) / 8.0;
	float d = nDotL * (1.0 - k) + k;
	return nDotL / d;
}

vec3 microfacetModel(vec3 v, vec3 n, vec3 lightCol, vec3 l, vec3 dif, float rough, float metal)
{
	vec3 h = normalize(v + l);
	float nDotH = clamp(dot(n, h), 0.0, 1.0);
	float vDotH = clamp(dot(v, h), 0.0, 1.0);
	float nDotL = clamp(dot(n, l), 0.0, 1.0);
	float nDotV = abs(dot(n, v)) + 1e-5;
	
	vec3 F = SchlickFresnel(vDotH, dif, metal);
	vec3 kD = 1.0 - F;
	vec3 specBRDF_nom = GgxDistribution(nDotH, rough) *
					F *
					GeomSmith(nDotL, rough) *
					GeomSmith(nDotV, rough);
	float specBRDF_denom = 4.0 * nDotV * nDotL + 1e-5;
	vec3 specBPDF = specBRDF_nom / specBRDF_denom;
	vec3 diffuseBRDF = kD * dif / PI;
	return (diffuseBRDF + specBPDF) * lightCol * nDotL;
}

float getShadowMulti(vec3 n, vec3 l, vec3 projCoords, int layer)
{
	if(projCoords.z > 1.0)
        return 0.0;
	
	float baseBias = max(0.05 * (1.0 - dot(n, l)), 0.005);
	float bias = baseBias * (cascadePlaneDist[layer] * 0.001);

	// PCF
	float shadow = 0.0;
	vec2 texelSize = 1.0 / vec2(textureSize(uShadowTex, 0));
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(uShadowTex, vec3(projCoords.xy + vec2(x,y) * texelSize, layer)).r;
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

float getShadowCubeMulti(vec3 n, vec3 l, float viewDist, float dist, int lightIdx, int numDirLights)
{
	vec3 fragToLight = -l;

	float bias = max(0.005 * (1.0 - dot(n, l)), 0.0005);
	float diskRadius = (1.0 + (viewDist / 20.0)) / 25.0;
	
	int samples = 20;
	float shadow = 0.0;

	vec3 noise = GetRandDir(vec3(gl_FragCoord.xy, 0.0));

	for(int i = 0; i < samples; ++i)
	{
		vec3 offset = reflect(gridSamplingDisk[i], normalize(noise));

		float closestDepth = texture(uShadowCubeMap, vec4(fragToLight + offset * diskRadius, float(lightIdx - numDirLights))).r;
		closestDepth *= uLight[lightIdx].uFarPlane;
		if(dist - bias > closestDepth)
			shadow += 1.0;
	}
	return shadow /= float(samples);
}

vec3 GetRandDir(vec3 seed)
{
	float j = 4096.0 * sin(dot(seed, vec3(17.0, 59.4, 15.0)));
	vec3 r;
	r.z = fract(512.0 * j);
	j *= .125;
	r.x = fract(512.0 * j);
	j *= .125;
	r.y = fract(512.0 * j);
	return r - 0.5;
}