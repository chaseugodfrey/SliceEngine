#ifndef ASSIMP_IMPORTER
#define	ASSIMP_IMPORTER

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <filesystem>
#include <iostream>
#include <fstream>

#include "Animation.h"

/*
* Note:
* if tryna build the compiler exe for compiling only, dont forget to remove the
* glew and glfw include dir and libs
*/
#define COMPILE_ONLY 1

#if! COMPILE_ONLY
#include <gl/glew.h>
#include "Shader.h"
#endif

namespace Geometry {
	/*
	* Mesh_Importer
	*/
	class Mesh_Compiler {
	public:
		bool compile_mesh{true};
		bool compile_animation{false};
		bool compile_material{false};
		/*
		* loads the asset file(.fbx, .obj etc) from src, and saves it as a.mdl file in dest
		* returns false if error occured
		*/
		bool Compile_Asset(const char* src, const char* dest);
	};


	/*
	* These classes are just for testing + drawing
	*/
	class Mesh;
	struct Texture;
	struct Bone;
	struct Vertex;


	class Model {
	public:

#if !COMPILE_ONLY
		void Init(const char* f) {
			load_model(f);
			init = true;
		}
		void InitLoadedModel();
		void Release();
		void Draw(GL_Shader&);

		/*
		 * used to keep track of bones that have been encountered
		 * kind of useless on its own, meant to sync the indexes with the animator class's finalmtx
		 * 
		 * also for now dont compile animations
		*/
		std::unordered_map<std::string, BoneInfo> bone_map;
#endif
		//Loads .mdl file
		bool LoadModelFromFile(const char* file);

		//Saves .mdl file
		void SaveModelToFile(const char* file);

		//Loads asset file
		bool load_model(const char*);

	private:
		bool init = false;
		std::vector<Mesh> meshes;
		std::string directory;
		std::string name;


		uint64_t get_model_size() const;
		void pack_data(char* const buffer, uint64_t & offset);
		void unpack_data(char* const buffer, uint64_t& offset);

		void process_node(aiNode* node, aiScene const* scene, aiMatrix4x4 parent_tform);
		Mesh process_mesh(aiMesh* mesh, aiScene const* scene);
		std::vector<Texture> load_material_texture(aiMaterial* mat,
			aiTextureType type, std::string name);

#if !COMPILE_ONLY
		void SetVertexBone(Vertex&, int, float);
		void ExtractBoneWeights(std::vector<Vertex>&, aiMesh*, const aiScene*);
#endif
	};


	//Setup for Model class
	struct Vertex {
		glm::vec3 position{};
		glm::vec3 normal{};
		glm::vec2 uv{};
		//glm::vec4 tangent{};
		//glm::vec4 bitangent{};	//prob can just calc this in gpu
#if !COMPILE_ONLY
		int boneIDs[MAX_BONE_INFLUENCE]{-1};
		float weights[MAX_BONE_INFLUENCE]{};
		void SetBonesDefault();
#endif
	};

	struct Texture {
		unsigned int id;
		//std::string type;
	};

	class Mesh {
	public:
		std::string name{};
		std::vector<Vertex> vertices{};
		std::vector<unsigned int> indices{};
		std::vector<Texture> textures{};

		glm::mat4 transform{};

		Mesh() {};
		Mesh(std::vector<Vertex>&& v, std::vector<unsigned int>&& i, std::vector<Texture>&& t) :
			vertices(v), indices(i), textures(t) {
#if !COMPILE_ONLY
			setup_mesh();
#endif
		}

#if !COMPILE_ONLY
		void Draw(GL_Shader&);
		void Init();
		void Release();
		void setup_mesh();
#endif
		uint64_t get_mesh_size() const;
		void unpack_data(char* const buffer, uint64_t& offset);
		void pack_data(char* const buffer, uint64_t& offset);
	private:
#if !COMPILE_ONLY
		unsigned int vao{}, vbo{}, ebo{};
#endif
	};

}

#endif