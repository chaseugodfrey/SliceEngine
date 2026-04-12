#version 460 core
//impact_Frag
layout (location=0) in vec2 vTexCoord;

layout (location=0)	out vec4 fFragColor; // location 0 is default GL_BACK_LEFT color buffer

layout (binding = 0) uniform sampler2D 	uPos;
layout (binding = 1) uniform sampler2D 	uNom;

const float PI = 3.14159265358979323846;

uniform mat4 uVP;
uniform vec3 impactPos;
uniform vec3 impactColor;
uniform vec3 impactColor2;
uniform float time;
uniform int isSmooth;
uniform float epilepsy;
uniform float impactAngle = 0.31;
uniform float noiseScale = 148.0;
uniform float secondNoiseScale = 21.0;

float SimpleNoise(vec2 UV, float Scale);

/***************************************************
* Out: fFragColor
*****************************************************/
void main(void){
	fFragColor = vec4(0.0);

    vec4 clipPos = uVP * vec4(impactPos, 1.0);
    vec2 ndcPos = clipPos.xy / clipPos.w;
    vec2 impactScreenUV = ndcPos * 0.5 + 0.5;
	vec2 offset = vec2(0.5) - impactScreenUV;
    
	vec2 screenPosCenterd = vTexCoord - vec2(0.5) + offset;
	
	float modifiedTime = time * epilepsy;
    if(isSmooth == 0)
        modifiedTime = floor(modifiedTime);

	float noiseInpt = atan(screenPosCenterd.x, screenPosCenterd.y) + PI;
	float linesExclaim = length(screenPosCenterd) * SimpleNoise(vec2(noiseInpt), noiseScale);
	float weakerExclaim = SimpleNoise(vec2(modifiedTime + linesExclaim), secondNoiseScale);

	float blendA = 0.0;
	if(weakerExclaim / 2.0 > 0.23)
		blendA = 1.0;
	
	vec3 wPos = texture(uPos, vTexCoord).xyz;
	vec3 wNom = texture(uNom, vTexCoord).xyz;
	float blendB = 0.0;
	if(dot(normalize(wPos - impactPos), wNom) > impactAngle) // facing
		blendB = 1.0;
	
    float blendAlpha = abs(blendB - blendA);

	fFragColor = vec4(vec3(blendAlpha) * impactColor + (1.0 - blendAlpha) * impactColor2, 1.0);
}


void Hash_Tchou_2_1_uint(uvec2 v, out uint o)
{
    v.y ^= 1103515245U;
    v.x += v.y;
    v.x *= v.y;
    v.x ^= v.x >> 5u;
    v.x *= 0x27d4eb2du;
    o = v.x;
}

void Hash_Tchou_2_1_float(vec2 i, out float o)
{
    uint r;
    uvec2 v = uvec2(ivec2 (round(i)));
    Hash_Tchou_2_1_uint(v, r);
    o = (r >> 8) * (1.0 / float(0x00ffffff));
}

float SimpleNoiseVec2(vec2 uv)
{
    vec2 i = floor(uv);
    vec2 f = fract(uv);
    f = f * f * (3.0 - 2.0 * f);
    uv = abs(fract(uv) - 0.5);
    vec2 c0 = i + vec2(0.0, 0.0);
    vec2 c1 = i + vec2(1.0, 0.0);
    vec2 c2 = i + vec2(0.0, 1.0);
    vec2 c3 = i + vec2(1.0, 1.0);
    float r0; Hash_Tchou_2_1_float(c0, r0);
    float r1; Hash_Tchou_2_1_float(c1, r1);
    float r2; Hash_Tchou_2_1_float(c2, r2);
    float r3; Hash_Tchou_2_1_float(c3, r3);
    float bottomOfGrid = mix(r0, r1, f.x);
    float topOfGrid = mix(r2, r3, f.x);
    float t = mix(bottomOfGrid, topOfGrid, f.y);
    return t;
}

float SimpleNoise(vec2 UV, float Scale)
{
    float freq, amp;
    float Out = 0.0f;
    freq = pow(2.0, 0.0);
    amp = pow(0.5, 3.0);
    Out += SimpleNoiseVec2(vec2(UV.xy*(Scale/freq)))*amp;
    freq = pow(2.0, 1.0);
    amp = pow(0.5, 2.0);
    Out += SimpleNoiseVec2(vec2(UV.xy*(Scale/freq)))*amp;
    freq = pow(2.0, 2.0);
    amp = pow(0.5, 1.0);
    Out += SimpleNoiseVec2(vec2(UV.xy*(Scale/freq)))*amp;

	return Out;
}