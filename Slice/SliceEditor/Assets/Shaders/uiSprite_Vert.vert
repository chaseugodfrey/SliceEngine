#version 460 core

layout (location=0) in vec3 aVertexPosition;
layout (location=2) in vec2 aTextureCoord;

layout (location=0) out vec2 vTextureCoord;

uniform mat4 canvas_to_ndc;		//from canvas
uniform mat4 M;					//from recttransform
uniform vec4 uv = vec4(0.0, 1.0, 0.0, 1.0);


void main(void){
	vTextureCoord 	= aTextureCoord;
	vTextureCoord.x *= (uv[1] - uv[0]);
	vTextureCoord.x += uv[0];
	vTextureCoord.y *= (uv[3] - uv[2]);
	vTextureCoord.y += uv[2];


	gl_Position	=  canvas_to_ndc * M * vec4(aVertexPosition, 1.0);
	//gl_Position = vec4(aVertexPosition, 1.0);
}