#ifndef MODEL_H
#define MODEL_H

#include <glm/glm.hpp>
#include <vector>
#include <GL/glew.h>
#include <string>

namespace SliceEngine
{
	namespace SliceEngineTypes
	{
		class Model {
			//ngl no idea whr to put this so ill put this for now
		public:
			/*static unsigned int LoadTexture(std::string const&);

			unsigned int texture_id{};*/

			std::vector<glm::vec3> vtx;
			GLuint vao{}, vbo{};
			GLuint drawCnt{};
			GLenum drawMode{};

			static Model LoadModel(std::string const&);
			void DestroyModel();

		};
	}
}

#endif
