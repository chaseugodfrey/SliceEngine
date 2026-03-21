#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;

uniform int uLightIdx;

layout (location=0)	out vec4 gFragPos; // FragPos from GS (output per emitvertex)

void main()
{
    gl_Layer = uLightIdx; // built-in variable that specifies to which face we render.
    for(int i = 0; i < 3; ++i) // for each triangle's vertices
    {
        gFragPos = gl_in[i].gl_Position;
        gl_Position = gFragPos;
        EmitVertex();
    }    
    EndPrimitive();
} 