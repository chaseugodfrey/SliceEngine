#version 460 core

layout (location=0) in vec3 vTexCoord;

layout (location=0)	out vec4 fFragColor;

const float liteSunConvergence = 4.5;
const float liteCloudLightInt  = 1.0;
const vec3 gamma = vec3(0.45454545454);

uniform vec3 sunPos = vec3(0.0, 1.0, 0.0);
uniform vec3 sunCol = vec3(1.0, 1.0, 1.0);
uniform float sunSize = 0.507;					// (0, 1)
uniform float sunHaze = 0.161;					// (0, 0.5)
uniform float atmosphereThickness = 1.2;		// (0.1, 5)
uniform vec4 zenithColor = vec4(0.733, 0.87, 1.0, 1.0);
uniform vec4 horizonColor = vec4(0.815, 0.917, 1.0, 1.0);
uniform vec3 groundColor = vec3(1, 1, 1);
uniform float skyExposure = 0.54;				// (0,4)
uniform float horizonHeight = -0.031;				// (-0.5, 0.5)
uniform vec4 cloudTint = vec4(1.0, 1.0, 1.0, 1.0);
uniform float cloudSeed = 0.0;					// (0, 1000)
uniform float cloudRotationY = 204.0;			// (0, 360)
uniform float cloudCoverage = 1.0;				// (0, 1)
uniform float cloudSoftness = 0.406;				// (0.2, 1)
uniform float cloudScale = 2;					// (1, 2)
uniform float cloudBaseHeight = 0.0;			// (0, 0.5)

// ----- Skybox Generation -----
vec3 ProcessSky(vec3 dir);
vec4 ProcessClouds(vec3 nom, vec3 sky);
vec3 ProcessSun(vec3 nom);

// ----- General Functions -----
float satf(float x);
vec3 satv3(vec3 x);
float discFade_HQ(float d, float s);
vec3 hash33_fast(vec3 p);
float has31_fast(vec3 p);
float sphereFalloff(float d, float s);
float fbm3(vec3 p);
float noise3(vec3 p);
float blobDensity(vec3 localPos, vec3 cellSeed);
vec4 linearToGamma(vec4 linearRGB);
vec3 GammaToLinear(vec3 sRGB);

void main(void){
	vec3 nom = normalize(vTexCoord);

	vec3 sky = ProcessSky(nom);

	vec4 clouds = ProcessClouds(nom, sky);
	vec3 result = mix(sky, clouds.rgb, clouds.a);

	result += ProcessSun(nom) * (1.0 - clouds.a);
	//result += ProcessSun(nom) * (1.0 - clouds.a); // Idk why duplicate this

	result *= skyExposure;

	fFragColor = vec4(satv3(result), 1.0);
}

// ----- Skybox Generation -----

vec3 ProcessSky(vec3 dir)
{
	float y			= satf(dir.y * 0.5 + 0.5) * 2.0 - 1.0;
	float yShift	= y - horizonHeight;

	float ySky		= satf(yShift);
	float yGround	= satf(-yShift);

	float tSky		= pow(ySky, atmosphereThickness);
	float tGround	= yGround;

	vec3 top		= mix(horizonColor.rgb, zenithColor.rgb, tSky);
	vec3 bot		= mix(groundColor.rgb, horizonColor.rgb, tGround);

	float blend		= smoothstep(-0.02, 0.02, yShift);
	return mix(bot, top, blend);
}

vec4 ProcessClouds(vec3 nom, vec3 sky)
{
	float rad = radians(cloudRotationY);
	float s = sin(rad), c = cos(rad);
	vec3 rotDir = vec3(nom.x * c - nom.z * s, nom.y, nom.x * s + nom.z * c);

	float baseOffset = rotDir.y - cloudBaseHeight;
	if(baseOffset <= 0.0)
		return vec4(sky, 0.0);

	vec3 pos = vec3(rotDir.x, baseOffset, rotDir.z) * cloudScale;
	rotDir = normalize(rotDir);

	vec2 camXZ = vec2(0.0); // Meant to be worldSpaceCamPos
	pos.xz += camXZ * (0.001 * cloudScale);

	const float cellSize	= 1.5;
	const int	layers		= 3;
	const float	step		= 0.6;

	float density = 0.0;
	for(int l = 0; l < layers; ++l)
	{
		vec3 lp		= pos + rotDir * (float(l - 1) * step);

		vec3 gPos	= lp / cellSize;
		vec3 b		= floor(gPos);
		vec3 f		= gPos - b;

		const float maxLayerOffset	= step;
		const float planeMargin		= max(0.12, maxLayerOffset / cellSize * 0.55);

		ivec3 minC = ivec3(b) - ivec3(1);
		ivec3 maxC = ivec3(b) + ivec3(1);

		if(f.x < planeMargin)
			minC.x -= 1;
		else if(f.x > 1.0 - planeMargin)
			maxC.x += 1;
		if(f.y < planeMargin)
			minC.y -= 1;
		else if(f.y > 1.0 - planeMargin)
			maxC.y += 1;
		if(f.z < planeMargin)
			minC.z -= 1;
		else if(f.z > 1.0 - planeMargin)
			maxC.z += 1;

		float d = 0.0;
		const float maxR = 1.0 + cloudSoftness;
		const float maxR2 = maxR * maxR;

		for(int ix = minC.x; ix <= maxC.x; ++ix)
		{
			for(int iy = minC.y; iy <= maxC.y; ++iy)
			{
				for(int iz = minC.z; iz <= maxC.z; ++iz)
				{
					vec3 cell = vec3(float(ix), float(iy), float(iz));
					vec3 randOffset = hash33_fast(cell) - vec3(0.5);
					vec3 centre = (cell + randOffset) * cellSize;
					vec3 dv = lp - centre;

					if(dot(dv, dv) > maxR2)
						continue;
					d += blobDensity(dv, cell);
				}
			}
		}
		density += d;
	}
	density = satf(density / layers);
	density = satf(density - (1.0 - cloudCoverage));

	float heightFade = satf(baseOffset / 0.15);
	density *= heightFade;

	float densOver = density;

	vec3 sDir		= normalize(sunPos);
	float mu		= satf(dot(nom, sDir));

	vec3 lightCol	= GammaToLinear(sunCol);
	float sunVisable = satf(sDir.y * 0.5 + 0.5);
	
	float g = mix(0.3, 0.78, sunVisable);
	float denom = 1.0 + g * g - 2.0 * g * mu;
	float phase = (1.0 - g * g) / max(pow(denom, 1.5), 1e-3);

	vec3 tint = cloudTint.rgb;
	vec3 tintedLight = lightCol * tint;

	float transV = exp(-densOver * 1.35);
	vec3 scatter = tintedLight * phase * liteCloudLightInt * (1.0 - transV) * sunVisable;

	float silver = pow(satf(1.0 - densOver), 3.0) * pow(mu, 8.0) * (liteCloudLightInt * 0.35) * sunVisable;
	vec3 silverCol = tintedLight * silver;

	vec3 outCol = mix(sky, tint, densOver);
	outCol = satv3(outCol + scatter * densOver + silverCol);

	return vec4(outCol, densOver);
}

vec3 ProcessSun(vec3 nom)
{
	if(sunSize <= 0.0)
		return vec3(0.0);

	vec3 sDir	= normalize(sunPos);
	
	float d		= length(nom - sDir);

	float core	= discFade_HQ(d, sunSize);
	float u		= satf(d / max(sunSize, 1e-4));
	float limb	= pow(1.0 - u, 0.35);
	core *= limb;

	float hazeR	= max(sunSize * mix(1.5, 5.0, satf(sunHaze * 2.0)), sunSize + 1e-4);
	float x	= max(d - sunSize, 0.0) / (hazeR - sunSize);
	float halo = exp2(-4.0 * x * x);

	vec3 L = GammaToLinear(sunCol);
	vec3 coreCol = L * vec3(1.08, 0.85, 0.9);
	vec3 haloCol = L * vec3(1.0, 0.92, 0.85);

	float hazeStrength = mix(0.5, 3.0, satf(sunHaze * 2.0));
	vec3 col = coreCol * core + haloCol * halo * hazeStrength;

	return satv3(col);
}

// ----- General Functions -----
float satf(float x)
{
	return clamp(x, 0.0, 1.0);
}

vec3 satv3(vec3 x)
{
    return clamp(x, vec3(0.0), vec3(1.0));
}

float discFade_HQ(float d, float s)
{
	float conv = liteSunConvergence;
	float edge = max(s / (conv * 6.0), 1e-4);
	return 1.0 - smoothstep(s - edge, s, d);
}

vec3 hash33_fast(vec3 p)
{
	p += vec3(cloudSeed);
	p = fract(p * 0.1031);
	p += dot(p, p.yxz + vec3(33.33));
	return fract((p.xxy + p.yzz) * p.zyx);
}
float has31_fast(vec3 p)
{
	p += cloudSeed;
	p = fract(p * 0.1031);
	p += dot(p, p.yxz + vec3(33.33));
	return fract((p.x + p.y) * p.z);
}
float fbm3(vec3 p)
{
	float v = 0.0;
	float a = 0.5;
	for(int i = 0; i < 4; ++i)
	{
		v += noise3(p) * a;
		p *= 2.0;
		a *= 0.5;
	}
	return v;
}
float noise3(vec3 p)
{
	vec3 i = floor(p);
	vec3 f = fract(p);
	vec3 u = f * f * (3.0 - 2.0 * f);

	float n000 = has31_fast(i + vec3(0,0,0));
	float n100 = has31_fast(i + vec3(1,0,0));
	float n010 = has31_fast(i + vec3(0,1,0));
	float n110 = has31_fast(i + vec3(1,1,0));
	float n001 = has31_fast(i + vec3(0,0,1));
	float n101 = has31_fast(i + vec3(1,0,1));
	float n011 = has31_fast(i + vec3(0,1,1));
	float n111 = has31_fast(i + vec3(1,1,1));

	return mix(
		mix(mix(n000, n100, u.x), mix(n010, n110, u.x), u.y),
		mix(mix(n001, n101, u.x), mix(n011, n111, u.x), u.y),
		u.z
	);
}
float sphereFalloff(float d, float s)
{
	float k = satf((1.0 - d) / max(s, 1e-3));
	return k * k * (3.0 - 2.0 * k);
}

float blobDensity(vec3 localPos, vec3 cellSeed)
{
	float s = sphereFalloff(length(localPos), cloudSoftness);
	float n = fbm3(localPos * 2.0 + cellSeed * 5.37);
	n = satf((n - 0.3) * 2.0);
	return s * n;
}

vec4 linearToGamma(vec4 linearRGB)
{
    bvec3 cutoff = lessThan(linearRGB.rgb, vec3(0.0031308));
    vec3 higher = vec3(1.055)*pow(linearRGB.rgb, vec3(1.0/2.4)) - vec3(0.055);
    vec3 lower = linearRGB.rgb * vec3(12.92);

    return vec4(mix(higher, lower, cutoff), linearRGB.a);
}

vec3 GammaToLinear(vec3 sRGB)
{
    bvec3 cutoff = lessThan(sRGB, vec3(0.04045));
    vec3 higher = pow((sRGB + vec3(0.055))/vec3(1.055), vec3(2.4));
    vec3 lower = sRGB.rgb/vec3(12.92);

    return mix(higher, lower, cutoff);
}