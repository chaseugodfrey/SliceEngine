#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

namespace SliceEngine
{
	struct Shader
	{
		GLuint s;
	};

	struct Model
	{
		std::vector<glm::vec3> vtx;
		GLuint vao{}, vbo{};
		GLuint drawCnt{};
		GLenum drawMode{};
	};

	class ResourceManager
	{
	public:
		ResourceManager();
		~ResourceManager();

		void LoadShader(const std::string& vertFile, const std::string& fragFile);
		void LoadModel(const std::string& file);

		Shader& GetShader();
		Model& GetModel();
	private:
		Shader mOnlyShader;
		Model mOnlyModel;
	};
}


#endif