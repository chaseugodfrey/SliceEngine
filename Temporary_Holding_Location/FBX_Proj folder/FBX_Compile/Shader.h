#ifndef SHADER
#define	SHADER

#include "Assimp_Importer.h"
#if !COMPILE_ONLY
struct GL_Shader {
	unsigned int handle;

	void Compile_Link_Validate(const char* v, const char* f);
	void Destroy();

	unsigned int GetUniformLoc(const char*) const;
};
#endif

#endif