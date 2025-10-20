#version 460 core

const vec3 cIdxPos[4] = vec3[4](
	vec3(-1.0, 0.0, -1.0),
	vec3( 1.0, 0.0, -1.0),
	vec3( 1.0, 0.0,  1.0),
	vec3(-1.0, 0.0,  1.0)
);


const int cIndices[6] = int[6](0,2,1,2,0,3);

layout (location=0) out vec3 vPos;

const float cPlaneSize = 250.0;

uniform mat4 V; // View transform matrix
uniform mat4 P; // Perspective transform matrix
uniform vec2 uCamPos;


void main(void){
	vPos = cIdxPos[cIndices[gl_VertexID]] * cPlaneSize;
	
	vPos.x += uCamPos.x;
	vPos.z += uCamPos.y;
	
	gl_Position = P * V * vec4(vPos, 1.0);
}
