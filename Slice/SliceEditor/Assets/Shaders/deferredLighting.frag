#version 460 core

struct Light{
	vec3 position;
	vec3 La;
	vec3 Ld;
	vec3 Ls;
};

struct Material{
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float shininess;
};

layout (location=0) in vec3 vPos; // In MV Space
layout (location=1) in vec3 vNom; // In MV Space
layout (location=2) in vec2 vTexCoord;
layout (location=3) in flat uint vGID;

layout (location=0)	out vec4 fFragColor; // location 0 is default GL_BACK_LEFT color buffer
layout (location=1) out uint fGID;
layout (location=2) out vec3 fPositionData;
layout (location=3) out vec3 fNormalData;

uniform int uPass;
uniform Material uMat;
uniform Light uLight[1];
uniform mat4 V;

layout (binding = 0) uniform sampler2D 	uTex;
layout (binding = 1) uniform sampler2D 	uPosTex;
layout (binding = 2) uniform sampler2D 	uNomTex;
// if doing instance rendering, save bindings 12~15 // could lower to 13~15

vec3 BlinnPhong(vec3 pos, vec3 nom, Light light, Material mat){
	vec3 color = vec3(0.f, 0.f, 0.f);

	if(any(notEqual(nom, vec3(0.0f))))// Not Background
	{
		color = light.La * mat.Ka; // ambient
		vec3 lightPosView = vec3(V * vec4(light.position, 1.0f));
		vec3 toLight = lightPosView - pos;
		if(length(toLight) > 0.f)
		{
			toLight = normalize(toLight);

			float cosTheta = max(dot(toLight, nom), 0.0f);
			vec3 diffuse = light.Ld * mat.Kd * cosTheta;

			vec3 specular = vec3(0.0f);
			// Don't need calculate specular if angle is >90
			if(cosTheta > 0.0f)
			{
				pos = normalize(-pos);
				vec3 h = normalize(pos + toLight);
				float cosPhi = max(dot(h, nom), 0.0f);
				specular = light.Ls * mat.Ks * pow(cosPhi, mat.shininess);
			}
			color += diffuse + specular;
		}
	}
	return color;
}

/***************************************************
* Out: fPositionData, fNormalData
*****************************************************/
void Pass0(){
	fPositionData = vPos;
	fNormalData = normalize(vNom);
	fGID = vGID;
}

/***************************************************
* Out: fFragColor
*****************************************************/
void Pass1(){
	vec3 pos = vec3(texture(uPosTex, vTexCoord));
	vec3 nom = vec3(texture(uNomTex, vTexCoord));
	if(any(notEqual(nom, vec3(0.0f))))
		fFragColor = vec4(BlinnPhong(pos, nom, uLight[0], uMat), 1.0f);
	else
		fFragColor = vec4(0.75294f, 1.0f, 0.93333f, 1.0f);
}

void main(void){
	if		(uPass == 0) Pass0();
	else if (uPass == 1) Pass1();
}
