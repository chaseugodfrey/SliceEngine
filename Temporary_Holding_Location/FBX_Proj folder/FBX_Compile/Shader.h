#ifndef SHADER
#define	SHADER

struct GL_Shader {
	unsigned int handle;

	void Compile_Link_Validate(const char* v, const char* f);
	void Destroy();

	unsigned int GetUniformLoc(const char*) const;
};

#endif