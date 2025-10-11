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
		//forward declare of helper struct
		struct Mesh;
		struct Vertex;

		class Model {
			//ngl no idea whr to put this so ill put this for now
			std::vector<Vertex> final_vert{};
			std::vector<unsigned int> final_ind{};
		public:

			std::vector<Vertex> GetFinalVert();
			std::vector<unsigned int> GetFinalInd();
			/*
				old ones
			*/
			std::vector<glm::vec3> vtx;

			/*
			* new ones
			*/
			void unpack_data(char* const buffer, uint64_t& offset);	//helper func
			GLuint ebo{};
			std::string name{};	//this can prob help with some editor stuff? gona leave it here for now, ez to remove later

			void combine_setup_meshes(std::vector<Mesh> const& meshes);


			/*
			* no change
			*/
			GLuint vao{}, vbo{};
			GLuint drawCnt{};
			GLenum drawMode{};
			//cant be static because of gl funcs i think
			void LoadDefaultCubeModel();
			void LoadDefaultLineModel();
			void LoadDefaultFrustrumModel();
			void LoadDefaultQuadModel();
			bool LoadModelResource(std::string const&);	//this one is new one
			void DestroyModel();
		};

		//helper structs - format of the resource file
		struct Vertex {
			glm::vec3 position{};
			glm::vec3 normal{};
			glm::vec2 uv{};
		};
		struct ResTexture {
			unsigned int id;
		};
		struct Mesh {
			std::string name{};
			std::vector<Vertex> vertices{};
			std::vector<unsigned int> indices{};
			//supposed to have material in here too
			std::vector<ResTexture> textures{};

			glm::mat4 transform{};
			void unpack_data(char* const buffer, uint64_t& offset);
		};
	}
}

#endif
