#version 460 core
//lighting_Frag
struct Light{
	vec3 position;
	float uFarPlane;
	vec3 direction;
	int type;
	vec4 color; // rgb + intensity
	int hasShadow;
	int shadowNum;
	int spotShadowNum;
	float pointAngle;
	mat4 VP;
};

layout (location=0)	out vec4 fFragColor; // location 0 is default GL_BACK_LEFT color buffer

const float PI = 3.14159265358979323846;
const float EPSILON = 0.000001;
// -TODO- Temporary material values
const float biasModifier = 0.5f;
const int isDirectional = 0;
const int isPoint 		= 1;
const int isSpot 		= 2;

layout (std140, binding = 0) uniform lightSpaceBlock
{
	mat4 lightSpaceMtx[16];
};
layout (std140, binding = 1) uniform lights
{
	Light uDirectionLight;
	Light uLight[150];
};

uniform mat4 V;
uniform int numLights;
uniform int hasDirectionalLight;
uniform vec3 uCamPos;
uniform float cascadePlaneDist[16];
uniform int cascadeCnt;

layout (binding = 0) uniform sampler2D 	uTex;
layout (binding = 1) uniform sampler2D 	uPosTex;
layout (binding = 2) uniform sampler2D 	uNomTex;
layout (binding = 3) uniform sampler2D 	uRoughMetalLightTex;
layout (binding = 4) uniform sampler2DArray uShadowTex;			// Only for shadow mapping (spot / directional light)
layout (binding = 5) uniform samplerCubeArray 	uShadowCubeMap; // Only for shadow mapping (point light)

float getShadowMulti(vec3 n, vec3 l, vec3 projCoords, int layer);
float getShadowSideMulti(vec3 n, vec3 l, float dist, int lightIdx);
float getShadowCubeMulti(vec3 n, vec3 l, float viewDist, float dist, int lightIdx);
vec3 microfacetModel(vec3 v, vec3 n, vec3 lightCol, vec3 l, vec3 dif, float rough, float metal);
vec3 GetRandDir(vec3 seed);

/***************************************************
* Out: fFragColor (Addictive)
*****************************************************/
void main(void){
	ivec2 p = ivec2(gl_FragCoord.xy);
	vec3 wPos = texelFetch(uPosTex, p, 0).xyz;// In World Space
	vec3 nom = texelFetch(uNomTex, p, 0).xyz;
	vec4 dif = texelFetch(uTex, p, 0);
	vec2 roughMetal = texelFetch(uRoughMetalLightTex, p, 0).xy;
	float toUseLight =  texelFetch(uRoughMetalLightTex, p, 0).z;

	fFragColor = vec4(0.0);

	if(any(notEqual(nom, vec3(0.0f))) && abs(dif.a) > EPSILON && toUseLight < 0.5f)
	{
		nom = normalize(nom);
		vec3 v = normalize(-wPos);
		
		if(hasDirectionalLight != 0)
		{
			vec4 fragViewSpace = V * vec4(wPos, 1.0f);
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

			vec4 vLightPos = lightSpaceMtx[layer] * vec4(wPos, 1.0f);
			vec3 projCoords = vLightPos.xyz / vLightPos.w;
			projCoords = projCoords * 0.5f + 0.5f;

			vec3 finalLighting = vec3(0.0f); // if blocked by shadow

			vec3 l = normalize(-uDirectionLight.direction);// Surface to Light
			float shadow = uDirectionLight.hasShadow * getShadowMulti(nom, l, projCoords, layer);
			finalLighting += (1.0 - shadow) * microfacetModel(v, nom, uDirectionLight.color.rgb * uDirectionLight.color.a, l, dif.rgb, roughMetal.x, roughMetal.y);
			fFragColor += vec4(finalLighting, 0.0f);
		}
		for(int lightIdx = 0; lightIdx < numLights; ++lightIdx)
		{
			if(uLight[lightIdx].type == isPoint)
			{
				vec3 l = uLight[lightIdx].position - uCamPos - wPos; // Surface to Light
				float dist = max(length(l), 0.001);
				vec4 lightCol = uLight[lightIdx].color;
				lightCol.a /= (dist * dist); // Insensity is normalized, so scale up by 100?

				float shadow = uLight[lightIdx].hasShadow * getShadowCubeMulti(nom, l, length(wPos), dist, lightIdx);
				l = l / dist;
				fFragColor += vec4(((1.0 - shadow) * microfacetModel(v, nom, lightCol.rgb * lightCol.a, l, dif.rgb, roughMetal.x, roughMetal.y)), 0.0f);
			}
			else if(uLight[lightIdx].type == isSpot)
			{
				vec3 l = uLight[lightIdx].position - uCamPos - wPos;
    			float dist = max(length(l), 0.001);
    			vec3 L = l / dist;
    			vec3 lightDir = normalize(uLight[lightIdx].direction); // Spotlight's forward direction
    			vec3 fragToLight = -L;

				float theta = dot(lightDir, fragToLight);
    			float cutOff = cos(float(uLight[lightIdx].pointAngle) / 2.0);

				float epsilon = 0.05;
    			float intensity = clamp((theta - cutOff) / epsilon, 0.0, 1.0);

				if (intensity > 0.0) {
					vec4 lightCol = uLight[lightIdx].color;
					lightCol.a = lightCol.a / (dist * dist) * intensity;

					float shadow = uLight[lightIdx].hasShadow * getShadowSideMulti(nom, l, dist, lightIdx);
					fFragColor += vec4(((1.0 - shadow) * microfacetModel(v, nom, lightCol.rgb * lightCol.a, L, dif.rgb, roughMetal.x, roughMetal.y)), 0.0f);
    			}
			}
		}
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

vec3 microfacetModel(vec3 v, vec3 n, vec3 lightCol, vec3 l, vec3 dif, float rough, float metal)
{
	vec3 h = normalize(v + l);
	float nDotH = clamp(dot(n, h), 0.0, 1.0);
	//float lDotH = clamp(dot(l, h), 0.0, 1.0);
	float vDotH = clamp(dot(v, h), 0.0, 1.0);
	float nDotL = clamp(dot(n, l), 0.0, 1.0);
	float nDotV = abs(dot(n, v)) + 1e-5;
	
	//vec3 specBrdf = 0.25f * GgxDistribution(nDotH, rough) * SchlickFresnel(lDotH, dif, metal) *  GeomSmith(nDotL, rough) * GeomSmith(nDotV, rough);
	//return (dif + PI * specBrdf) * lightCol * nDotL;

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
 	// Because I forced the minZ & maxZ to be huge
	//float bias = max(0.001 * (1.0 - dot(n, l)), 0.0001);

	//float bias = max(0.05 * (1.0 - dot(n, l)), 0.005); 
	////bias *= (cascadePlaneDist[layer] / uFarPlane);

	//if(layer == cascadeCnt)
	//{
	//	bias *= 1 / (uFarPlane * biasModifier);
	//}
	//else
	//{
	//	bias *= 1 / (cascadePlaneDist[layer] * biasModifier);
	//}

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

float getShadowCubeMulti(vec3 n, vec3 l, float viewDist, float dist, int lightIdx)
{
	vec3 fragToLight = -l;
	vec3 L = normalize(l);

	float bias = max(0.05 * (1.0 - dot(n, L)), 0.005) * (uLight[lightIdx].uFarPlane / 20.0);
	float diskRadius = (1.0 + (viewDist / 20.0)) / 25.0;
	
	int samples = 20;
	float shadow = 0.0;

	vec3 noise = GetRandDir(vec3(gl_FragCoord.xy, 0.0));

	for(int i = 0; i < samples; ++i)
	{
		vec3 offset = reflect(gridSamplingDisk[i], normalize(noise));

		//float closestDepth = texture(uShadowCubeMap, fragToLight + offset * diskRadius).r;
		float closestDepth = texture(uShadowCubeMap, vec4(fragToLight + offset * diskRadius, float(uLight[lightIdx].shadowNum))).r;
		closestDepth *= uLight[lightIdx].uFarPlane;
		if(dist - bias > closestDepth)
			shadow += 1.0;
	}
	return shadow /= float(samples);
}

vec3 UVToCubeDir(vec2 uv, int face) 
{
    vec2 c = uv * 2.0 - 1.0;
    if (face == 0) return vec3(1.0, -c.y, -c.x);       // +X
    if (face == 1) return vec3(-1.0, -c.y, c.x);       // -X
    if (face == 2) return vec3(c.x, 1.0, c.y);         // +Y
    if (face == 3) return vec3(c.x, -1.0, -c.y);       // -Y
    if (face == 4) return vec3(c.x, -c.y, 1.0);        // +Z
    return vec3(-c.x, -c.y, -1.0);                     // -Z
}

float getShadowSideMulti(vec3 n, vec3 l, float dist, int lightIdx) // Removed wPos from parameters!
{
    vec3 L = normalize(l); 
    
    // -l is EXACTLY the Fragment's position relative to the Light!
    // Multiply this directly by the Spotlight's VP matrix
    vec4 vLightPos = uLight[lightIdx].VP * vec4(-l, 1.0);
    
    // Prevent reverse-projection artifacts if the fragment is behind the spotlight
    if (vLightPos.w <= 0.0) return 1.0; 
    
    vec3 projCoords = vLightPos.xyz / vLightPos.w;
    projCoords = projCoords * 0.5 + 0.5;

    // Out of bounds check
    if(projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
    {
        return 1.0; 
    }

    // Using radial linear distance to match the spherical depth stored in your cubemap
    float currentDepth = dist / uLight[lightIdx].uFarPlane; 
    
    float bias = max(0.005 * (1.0 - dot(n, L)), 0.0005);
    float shadow = 0.0;
    
    vec2 texelSize = 1.0 / vec2(textureSize(uShadowCubeMap, 0).xy); 
    float layer = float(uLight[lightIdx].shadowNum); 
    int face = uLight[lightIdx].spotShadowNum;
	
    for(int x = -2; x <= 2; ++x)
    {
        for(int y = -2; y <= 2; ++y)
        {
            // Convert 2D PCF offsets into a 3D Cubemap vector
            vec2 offsetUV = projCoords.xy + vec2(x,y) * texelSize;
            vec3 cubeDir = UVToCubeDir(offsetUV, face);
            
            float pcfDepth = texture(uShadowCubeMap, vec4(cubeDir, layer)).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }

    return shadow / 25.0;
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