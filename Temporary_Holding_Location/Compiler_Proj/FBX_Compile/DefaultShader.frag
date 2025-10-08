#version 450 core

layout (location=0) in vec3 i_normal;
layout (location=1) in vec2 i_texCoord;
layout (location=2) in vec3 i_position;

uniform vec3 light_pos;
uniform mat4 V;         // View transform matrix
uniform sampler2D uTex2D;

layout (location=0) out vec4 fragColor;

void main(){
	float a_weight = 0.2;
	float d_weight = 0.5;
	float s_weight = 0.3;

	//fragColor = texture(uTex2D, i_texCoord);
	vec3 light_view = (V*vec4(light_pos, 1.0)).xyz;

	//ambient
	vec3 ambient = vec3(a_weight);
	//diffuse
	vec3 diffuse_color = vec3(1.0);
	vec3 norm = normalize(i_normal);
	vec3 lightDir = normalize(light_view - i_position);
	
	vec3 diffuse = max(dot(lightDir, norm), 0.0) * diffuse_color * d_weight;

	//specular
	vec3 viewDir = normalize(-i_position);
	vec3 h = normalize(viewDir + lightDir);
	float phi = max(0.0, dot(h, i_normal));
	//phi = pow(phi, material.shininess);
	vec3 specular = vec3(1.0) * phi * s_weight;

	fragColor = vec4(diffuse + ambient + specular, 1.0);
}