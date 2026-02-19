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
#include <glm/gtc/quaternion.hpp>

#include <vector>
//#include <GL/glew.h>
#include <string>

/*
* I think can prob set these hard limits for now
* according to quick google search, seems like usually max 4 bone per vert
* and 100 bones per model is prob good nuff
*
* Unity says more then 4 might cook performance
* and some forums usually dont mention more then 100 bones a model
*/
constexpr unsigned char MAX_BONES = 100;
constexpr unsigned char MAX_BONE_INFLUENCE = 4;

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
			//glm::mat4 local_transform{};
			std::vector<unsigned short> mesh_ref;
			std::vector<ModelNode> children;

			glm::vec3 position{ 0.0f, 0.0f, 0.0f };
			glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
			glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

			void unpack_data(char* const buffer, uint64_t& offset);
		};

		class Model {
		public:
			/*
				old ones
			*/
			//std::vector<glm::vec3> vtx;

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

			bool is_static{ true };

			//void combine_setup_meshes(std::vector<Mesh> const& meshes);

			//cant be static because of gl funcs i think
			void LoadDefaultCubeModel();
			void LoadDefaultSphereModel(int stackCount = 20, int sectorCount = 30);// stack = Horizontal slices (latitude), sector = Vertical slices (longitude)
			void LoadDefaultCapsuleModel();
			void LoadDefaultCylinderModel();
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
		struct VertexBone {
			int boneIDs[MAX_BONE_INFLUENCE]{ -1,-1,-1,-1 };
			float weights[MAX_BONE_INFLUENCE]{ 0.f,0.f,0.f,0.f };

			void SetVertexBone(int id, float weight);
		};

		struct Mesh {
			bool static_model{true};
			std::string name{};
			std::vector<Vertex> vertices{};
			std::vector<unsigned int> indices{};
			std::vector<VertexBone> vert_bones{};

			GLuint vao{}, vbo{};	//actual vbo vao and ebo to draw
			GLuint ebo{};
			GLuint vbbo{};			//vert bone buffer

			GLuint drawCnt{};
			GLenum drawMode{};
			void unpack_static_data(char const* const buffer, uint64_t& offset);
			void unpack_skin_data(char const* const buffer, uint64_t& offset);
			void setup_mesh();	//allocate the handles vao, vbo, ebo as well as set drawcnt and drawmode
		};
	}
}

#endif
