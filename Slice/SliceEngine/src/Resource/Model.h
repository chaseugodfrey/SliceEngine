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

		//Node that represents the scenegraph from the resource pov
		//used to create the actual scenegraph of entities when trying to create a model in the scene
		struct ModelNode {
			std::string name{};
			glm::mat4 local_transform{};
			std::vector<unsigned short> mesh_ref;
			std::vector<ModelNode> children;

			void unpack_data(char* const buffer, uint64_t& offset);
		};

		class Model {
		public:
			/*
				old ones
			*/
			std::vector<glm::vec3> vtx;

			/*
			* moved to mesh
			*/
			/*GLuint vao{}, vbo{};
			GLuint drawCnt{};
			GLenum drawMode{};
			GLuint ebo{};*/

			/*
			* new ones
			*/
			void unpack_data(char* const buffer, uint64_t& offset);	//helper func
			std::string name{};	//this can prob help with some editor stuff? gona leave it here for now, ez to remove later
			ModelNode rootNode;
			std::vector<Mesh> meshes;	//list of meshes, in the case of static model, max 1 mesh per material involved in the model

			//void combine_setup_meshes(std::vector<Mesh> const& meshes);

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

		struct Mesh {
			std::string name{};
			std::vector<Vertex> vertices{};
			std::vector<unsigned int> indices{};

			GLuint vao{}, vbo{};	//actual vbo vao and ebo to draw
			GLuint ebo{};
			GLuint drawCnt{};
			GLenum drawMode{};
			void unpack_data(char* const buffer, uint64_t& offset);
			void setup_mesh();	//allocate the handles vao, vbo, ebo as well as set drawcnt and drawmode
		};
	}
}

#endif
