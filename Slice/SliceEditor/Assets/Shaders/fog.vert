#version 460 core

const vec2 cIdxPos[4] = vec2[4](
	vec2(-1.0, -1.0),
	vec2( 1.0, -1.0),
	vec2( 1.0,  1.0),
	vec2(-1.0,  1.0)
);

const int cIndices[6] = int[6](0,1,2,2,3,0);

layout (location=0) out vec2 vTexCoord;

void main(void){
	vTexCoord = cIdxPos[cIndices[gl_VertexID]];
	gl_Position = vec4(vTexCoord, 0.0, 1.0);
	vTexCoord = (vTexCoord + vec2(1.0, 1.0)) / 2.0;
}
