#version 460 core

layout (location=0) in vec3		aVertexPosition;
layout (location=15) in vec3	iVals; // offset, scale, if rotate

uniform mat4 V; // View transform matrix
uniform mat4 P; // Perspective transform matrix

uniform vec2 uPosOffset;
uniform float uScale;

void main(void){
	vec3 temp = aVertexPosition;
	temp.x *= iVals.y; // [-100,100], [0], [0]
	if (iVals.z > 0.5) // Means rotate
	{
		// [offset * uScale], [0], [-100, 100] * uScale // Whole things + uPosOffset
		temp.z = temp.x * uScale + uPosOffset.y;// Offsets
		temp.x = iVals.x * uScale + uPosOffset.x;
	}
	else
	{
		// [-100, 100] * uScale , [0], [offset * uScale] // Whole thing + uPosOffset
		temp.x += temp.x * uScale + uPosOffset.x;// Offsets
		temp.z = iVals.x * uScale + uPosOffset.y;	
	}

	gl_Position	= P * V * vec4(temp, 1.0);
}