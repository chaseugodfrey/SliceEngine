#version 460 core
layout (triangles, invocations = 5) in;
layout (triangle_strip, max_vertices = 3) out;

layout (std140, binding = 0) uniform lightSpaceBlock
{
	mat4 lightSpaceMtx[16];
};

void main(void){
	for(int i = 0; i < 3; ++i)
	{
		gl_Position = lightSpaceMtx[gl_InvocationID] * gl_in[i].gl_Position;
		gl_Layer = gl_InvocationID;
		EmitVertex();
	}
	EndPrimitive();
}