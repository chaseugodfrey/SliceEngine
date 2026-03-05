#version 460 core

layout (location=0) in vec3	aVertexPosition;

layout (location=0) out vec3 vPos;
layout (location=1) out vec3 vNom;

uniform mat4 V; // View transform matrix
uniform mat4 P; // Perspective transform matrix
uniform float uTime;
uniform vec3 uCamPos;
uniform float uCloudsHeight;
uniform float uCloudsIntensity;
uniform float uCloudsSmoothness;

const float cloudsWidthMult = 5.0f;

float getWaveHeight(vec2 p, float t) {
    float wave1 = sin(p.x + t); 
    float wave2 = cos(p.y - t * 0.8); 
    float wave3 = sin((p.x + p.y) * 1.5 + t * 1.2) * 0.5; 
    float wave4 = cos((p.x - p.y) * 2.3 - t * 1.5) * 0.25; 
    return wave1 + wave2 + wave3 + wave4;
}

void main(void){
	vPos = vec3(aVertexPosition.x * cloudsWidthMult + uCamPos.x,
	uCloudsHeight,
	aVertexPosition.z * cloudsWidthMult + uCamPos.z);

	vec2 p = vec2(vPos.x, vPos.z) * uCloudsSmoothness;
    float t = uTime * uCloudsIntensity;

	float centerHeight = getWaveHeight(p, t);
	vPos.y += centerHeight;
	float epsilon = 0.05;

	float dxHeight = getWaveHeight(p + vec2(epsilon * uCloudsSmoothness, 0.0), t);
    float dzHeight = getWaveHeight(p + vec2(0.0, epsilon * uCloudsSmoothness), t);
	float dHdx = (dxHeight - centerHeight) / epsilon;
    float dHdz = (dzHeight - centerHeight) / epsilon;

	vNom = normalize(vec3(-dHdx, 1.0, -dHdz));
	gl_Position	= P * V * vec4(vPos, 1.0);
}