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
* 
* for now, 
*		release = no glew/glfw
*		debug = have glew/glfw
*/
#define COMPILE_ONLY 1

#if! COMPILE_ONLY
#include <gl/glew.h>
#include "Shader.h"
#endif

#include <json.hpp>

namespace Geometry {
	/*
	* FBX_Importer
	*/
	class FBX_Compiler {
		struct Mesh_Options {
			bool static_mesh{ false };
			bool import_normals{ true };
			bool import_tangents{ false };

		};
		struct Anim_Options {

		};


		bool mesh{ true };
		unsigned int postprocess_flags{ 0 };
		bool compile_mesh(const char* src, const char* dst);
		bool compile_anim(const char* src, const char* dst);
		void set_options(nlohmann::json const& options, Assimp::Importer&);
	public:


		/*
		* loads the asset file(.fbx, .obj etc) from src, and saves it as a.mdl file in dest
		* returns false if error occured
		*/
		bool Compile_Asset(const char* src, const char* dst, nlohmann::json const& options);

		enum Primitive_Type {
			Cube,
			Sphere,
			Plane
		};
		void Compile_Primitive(Primitive_Type type, const char* dst);
	};


	/*
	* These classes are just for testing + drawing
	*/
	class Mesh;
	struct Texture;
	struct Bone;
	struct Vertex;

	//struct to represent a node in the scene graph
	struct Node {
		std::string name{};        
		glm::vec3 position{ 0.0f, 0.0f, 0.0f };
		glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
		glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
		std::vector<unsigned short> mesh_ref;
		std::vector<Node> children;
	};

	class Model {
	public:

#if !COMPILE_ONLY
		void Init(const char* f);
		void InitLoadedModel();
		void Release();
		void Draw(GL_Shader&);
		void draw_node(GL_Shader&, Node&, glm::mat4 const& parent);

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
		bool load_model(const char*, const aiScene*);

		//List of functions to create primitive type
		void Create_Cube();
		void Create_Sphere();
		void Create_Plane();
	private:
		bool init = false;
		std::vector<Mesh> meshes;
		std::string name;
		Node root_node;

		uint64_t get_model_size() const;
		uint64_t get_node_size(Node const&) const;
		void pack_data(char* const buffer, uint64_t & offset);
		void pack_node_data(char* const buffer, uint64_t& offset, Node const& node);
		void unpack_data(char* const buffer, uint64_t& offset);
		void unpack_node_data(char* const buffer, uint64_t& offset, Node& node);

		void process_node(aiNode* node, aiScene const* scene, aiMatrix4x4 parent_tform, Node&);
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
		//int boneIDs[MAX_BONE_INFLUENCE]{-1};
		//float weights[MAX_BONE_INFLUENCE]{};
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

		//glm::mat4 transform{};

		Mesh() {};
		Mesh(std::vector<Vertex>&& v, std::vector<unsigned int>&& i, std::vector<Texture>&& t) :
			vertices(v), indices(i), textures(t) {
#if !COMPILE_ONLY
			setup_mesh();
#endif
		}

#if !COMPILE_ONLY
		void Init();
		void Release();
		void setup_mesh();unsigned int vao{}, vbo{}, ebo{};

#endif
		uint64_t get_mesh_size() const;
		void unpack_data(char* const buffer, uint64_t& offset);
		void pack_data(char* const buffer, uint64_t& offset);
	};

}

#endif