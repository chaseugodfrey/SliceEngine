/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Model.h
 author:		Elton leosantosa
 email:			leosantosa@digipen.edu
 brief:			Loads Models

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef MODEL_H
#define MODEL_H

#include <glm/glm.hpp>
#include <vector>
//#include <GL/glew.h>
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

			//cant be static because of gl funcs i think
			bool LoadModel(std::string const&);
			void DestroyModel();

		};
	}
}

#endif
