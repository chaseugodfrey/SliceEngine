#version 460 core

layout (location=0) in vec2 vTexCoord;

layout (location=0) out uint fFrag;

layout (binding = 0) uniform usampler2D uTex;

uniform int uWidth = 5;

void main(void){
	ivec2 pix = ivec2(gl_FragCoord.xy);

	uint sum = texelFetch(uTex, pix, 0).r;
	if(sum == 0)
	{
		for(int i = -uWidth; i <= uWidth; ++i)
		{
			if(i != 0)
			{
				for(int q = -uWidth; q <= uWidth; ++q)
				{
					if(q != 0)
						sum += texelFetchOffset(uTex, pix, 0, ivec2(q, i)).r;
				}	
			}
		}
		fFrag = sum;
	}
	else
		fFrag = 0;
}
