#version 460 core
//final_Frag
layout (location=0) in vec2 vTexCoord;

layout (location=0)	out vec4 fFragColor; // location 0 is default GL_BACK_LEFT color buffer

layout (binding = 0) uniform sampler2D 	uTex; // Already undergone the addition of all objects
layout (binding = 1) uniform sampler2D 	uAvgLumTex;
layout (binding = 2) uniform sampler2D 	uImpactTex;

uniform float impactBlend;
uniform float uExposure = 1.0;
uniform float uGamma = 0.45454545454;
uniform float White = 0.928;

// if doing instance rendering, save bindings 12~15 // could lower to 13~15

uniform mat3 rgb2xyz = mat3( 
  0.4124564, 0.2126729, 0.0193339,
  0.3575761, 0.7151522, 0.1191920,
  0.1804375, 0.0721750, 0.9503041 );

uniform mat3 xyz2rgb = mat3(
  3.2404542, -0.9692660, 0.0556434,
  -1.5371385, 1.8760108, -0.2040259,
  -0.4985314, 0.0415560, 1.0572252 );

/***************************************************
* Out: fFragColor
*****************************************************/
void main(void){
	vec3 hdrCol = texture(uTex, vTexCoord).rgb;
	vec3 impactCol = texture(uImpactTex, vTexCoord).rgb;

	float avgLum = texture(uAvgLumTex, vec2(0.0)).r;
	if(abs(avgLum) < 1e-5)
		avgLum = 0.001;

  // Convert to XYZ
	vec3 xyzCol = rgb2xyz * hdrCol;

  // Convert to xyY
  float xyzSum = xyzCol.x + xyzCol.y + xyzCol.z;
  vec3 xyYCol = vec3(xyzCol.x / xyzSum, xyzCol.y / xyzSum, xyzCol.y);

  // Apply the tone mapping operation to the luminance (xyYCol.z or xyzCol.y)
  float L = (uExposure * xyYCol.z) / avgLum;
  L = (L * (1 + L / (White * White))) / (1 + L);
  
  // Using the new luminance, convert back to XYZ
  xyzCol.x = (L * xyYCol.x) / (xyYCol.y);
  xyzCol.y = L;
  xyzCol.z = (L * (1 - xyYCol.x - xyYCol.y))/xyYCol.y;

	hdrCol = xyz2rgb * xyzCol;

	// Gamma Correction
  vec3 gamma = vec3(uGamma);
	hdrCol = pow(hdrCol, gamma); // Gamma Correction
	fFragColor = vec4(mix(hdrCol, impactCol, impactBlend), 1.0);
}
