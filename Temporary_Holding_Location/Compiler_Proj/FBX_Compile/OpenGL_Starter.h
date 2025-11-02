#ifndef STARTER
#define STARTER
#include "Assimp_Importer.h"

#if !COMPILE_ONLY
#include <glm/glm.hpp>
#include <vector>

#include "Shader.h"


class OpenGL_Starter {
	struct GL_Model {
		unsigned int primitive_type;
		unsigned int primitive_cnt;
		unsigned int draw_cnt;	//index/
		unsigned int vao_id;
		unsigned int vbo_id;
	};
	struct GL_Vertex {
		glm::vec3 position;
		glm::vec2 texCoord;
		//glm::vec3 color;
	};
public:
	void Init();
	void Update();
	void Terminate();
private:
	//GL_Model default_quad{};
	Geometry::Model mdl;
	Geometry::Model mdl_clone;
	Geometry::AnimationPackage anim_package;
	Geometry::AnimationPlayer animator;
	Geometry::Skeleton skeleton;


	Geometry::AnimationPackage anim_clone;
	Geometry::Skeleton skeleton_clone;

	GL_Shader default_shader;
};
#endif


#endif