#version 460 core

layout (location=0) in vec3 vPos; // In M Space
layout (location=1) in vec3 vNom; // In MV Space
layout (location=2) in vec2 vTex;
layout (location=3) in flat uint vGID;

layout (location=0)	out vec4 fFragColor; // location 0 is default GL_BACK_LEFT color buffer
layout (location=1) out uint fGID;
layout (location=2) out vec3 fPositionData;
layout (location=3) out vec3 fNormalData;
layout (location=4) out vec4 fMetalRoughData;

layout (binding = 0) uniform sampler2D 	uTex;
uniform float uRoughness;
uniform float uMetallic;

void main(void){
	fPositionData = vPos;
	fNormalData = normalize(vNom);
	fFragColor = texture(uTex, vTex);
	fGID = vGID;
	fMetalRoughData.xy = vec2(uRoughness, uMetallic);
}
