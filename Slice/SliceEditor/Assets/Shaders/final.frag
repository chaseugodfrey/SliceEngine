#version 460 core

layout (location=0)	out vec4 fFragColor; // location 0 is default GL_BACK_LEFT color buffer

layout (binding = 0) uniform sampler2D 	uTex; // Already undergone the addition of all objects

// if doing instance rendering, save bindings 12~15 // could lower to 13~15

/***************************************************
* Out: fFragColor
*****************************************************/
void main(void){
	ivec2 p = ivec2(gl_FragCoord.xy);
	vec4 col = texelFetch(uTex, p, 0);

	if(any(notEqual(col.rgb, vec3(0.0f))))
	{
		fFragColor = col;
	}
	else // background
	{
		fFragColor = vec4(0.75294f, 1.0f, 0.93333f, 1.0f);
	}
	fFragColor.rgb = pow(fFragColor.rgb, vec3(1.0f/2.2f));
}
