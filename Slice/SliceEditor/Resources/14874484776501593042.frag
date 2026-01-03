#version 460 core

layout (location=0) in vec3 vPos;

layout (location=0)	out vec4	fFragColor; // location 0 is default GL_BACK_LEFT color buffer

const float cMinPixelsBtwnCells = 2.0;
const float	cGridSize = 1.0;
const float cPlaneSize = 250.0;
const float cNumBtwnThick = 5.0;
const vec4 cGridColorThin = vec4(0.5, 0.5, 0.5, 0.5);
const vec4 cGridColorThick = vec4(0.0, 0.0, 0.0, 1.0);

uniform vec3 uCamPos;

void main(void){
	vec2 dvx = vec2(dFdx(vPos.x), dFdy(vPos.x));
	vec2 dvy = vec2(dFdx(vPos.z), dFdy(vPos.z));

	vec2 dudv = vec2(length(dvx), length(dvy));
	
	float l = length(dudv);
	float lod = max(0.0, (log(l * cMinPixelsBtwnCells / cGridSize) / log(cNumBtwnThick)) + 1.0);

	float LOD0 = cGridSize * pow(cNumBtwnThick, floor(lod));
	float LOD1 = LOD0 * cNumBtwnThick;
	float LOD2 = LOD1 * cNumBtwnThick;

	dudv *= 4.0;

	vec2 mod_div_dudv = mod(vPos.xz, LOD0) / dudv;
	vec2 temp = vec2(1.0) - (abs(clamp(mod_div_dudv, vec2(0.0), vec2(1.0)) * 2.0 - vec2(1.0)));
	float Lod0a = max(temp.x, temp.y);
	
	mod_div_dudv = mod(vPos.xz, LOD1) / dudv;
	temp = vec2(1.0) - (abs(clamp(mod_div_dudv, vec2(0.0), vec2(1.0)) * 2.0 - vec2(1.0)));
	float Lod1a = max(temp.x, temp.y);
	
	mod_div_dudv = mod(vPos.xz, LOD2) / dudv;
	temp = vec2(1.0) - (abs(clamp(mod_div_dudv, vec2(0.0), vec2(1.0)) * 2.0 - vec2(1.0)));
	float Lod2a = max(temp.x, temp.y);

	float LodFade = fract(lod);
	vec4 color;

	if(Lod2a > 0.0)
	{
		color = cGridColorThick;
		color.a *= Lod2a;
	}
	else if(Lod1a > 0.0)
	{
		color = mix(cGridColorThick, cGridColorThin, LodFade);
		color.a *= Lod1a;
	}
	else
	{
		color = cGridColorThin;
		color.a *= Lod0a * (1.0 - LodFade);
	}

	float opacityFalloff = 1.0 - (clamp(length(vPos.xz - uCamPos.xz), 0.0, 1.0) / cPlaneSize);
	
	color.a *= opacityFalloff;

	fFragColor = color;
}
