#version 460 core

layout (location=0) in vec3 aVertexPosition;
layout (location=2) in vec2 aTextureCoord;

layout (location=0) out vec2 vTextureCoord;

uniform mat4 canvas_to_ndc;		//from canvas
uniform mat4 M;					//from recttransform



void main(void){
	vTextureCoord 	= aTextureCoord;

	gl_Position	=  canvas_to_ndc * M * vec4(aVertexPosition, 1.0);
	//gl_Position = vec4(aVertexPosition, 1.0);
}