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

void _CheckGLError(const char* file, int line);

#define CheckGLError() _CheckGLError(__FILE__, __LINE__)
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


		bool static_mesh{ true };
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
	};


	/*
	* These classes are just for testing + drawing
	*/
	class Mesh;
	struct Bone;
	struct Vertex;
	struct VertexBone;

	//struct to represent a node in the scene graph
	struct Node {
		std::string name{};        
		glm::vec3 position{ 0.0f, 0.0f, 0.0f };
		glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
		glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
		std::vector<unsigned short> mesh_ref;
		std::vector<Node> children;

		glm::mat4 local_tform{};
	};

	class Model {
	public:
		bool is_static{ false };
#if !COMPILE_ONLY
		void InitLoadedModel();
		void Release();
		void Draw(GL_Shader&);
		void draw_node(GL_Shader&, Node&, glm::mat4 const& parent);
#endif
		//Loads .mdl file
		bool LoadModelFromFile(const char* file);

		//Saves .mdl file
		void SaveModelToFile(const char* file);

		//Loads asset file and target to skeleton
		bool load_static_model(const char*);
		bool load_skinned_model(const char*, Skeleton const&);

		Node root_node;

	private:
		bool init = false;
		std::vector<Mesh> meshes;
		std::string name;

		//void pack_data(char* const buffer, uint64_t& offset) const;
		//void unpack_data(char const* const buffer, uint64_t& offset);

		uint64_t get_model_size() const;
		uint64_t get_node_size(Node const&) const;

		void pack_data(char* const buffer, uint64_t & offset) const;
		void pack_node_data(char* const buffer, uint64_t& offset, Node const& node) const;
		void unpack_data(char const* const buffer, uint64_t& offset);
		void unpack_node_data(char const* const buffer, uint64_t& offset, Node& node);

		//void pack_skin_data(char* const buffer, uint64_t& offset) const;
		//void pack_skin_node_data(char* const buffer, uint64_t& offset, Node const& node) const;
		//void unpack_skin_data(char const* const buffer, uint64_t& offset);
		//void unpack_skin_node_data(char const* const buffer, uint64_t& offset, Node& node);

		void process_node(aiNode* node, aiScene const* scene, aiMatrix4x4 const& parent_tform, Node&);
		void process_skin_node(aiNode* node, aiScene const* scene, Skeleton const& skeleton, aiMatrix4x4 const&, Node&);
		Mesh process_mesh(aiMesh* mesh, aiScene const* scene);
		Mesh process_skin_mesh(aiMesh* mesh, aiScene const* scene, Skeleton const& skeleton);

	};


	//Setup for Model class
	struct Vertex {
		glm::vec3 position{};
		glm::vec3 normal{};
		glm::vec2 uv{};
		//glm::vec4 tangent{};
		//glm::vec4 bitangent{};	//prob can just calc this in gpu
	};

	struct VertexBone {
		int boneIDs[MAX_BONE_INFLUENCE]{ -1,-1,-1,-1 };
		float weights[MAX_BONE_INFLUENCE]{ 0.f,0.f,0.f,0.f };

		void SetVertexBone(int id, float weight);
	};

	class Mesh {
	public:
		std::string name{};
		std::vector<Vertex> vertices{};
		std::vector<VertexBone> vert_bones{};


		std::vector<unsigned int> indices{};

		//glm::mat4 transform{};

		Mesh() {};
		Mesh(std::vector<Vertex>&& v, std::vector<unsigned int>&& i, std::vector<VertexBone>&& t) :
			vertices(v), indices(i), vert_bones(t) {}

#if !COMPILE_ONLY
		void Release();
		void setup_mesh();
		
		unsigned int vao{}, vbo{}, ebo{}, vbbo{};

#endif
		uint64_t get_static_mesh_size() const;
		void unpack_static_data(char const* const buffer, uint64_t& offset);
		void pack_static_data(char* const buffer, uint64_t& offset) const;

		uint64_t get_skin_mesh_size() const;
		void unpack_skin_data(char const* const buffer, uint64_t& offset);
		void pack_skin_data(char* const buffer, uint64_t& offset) const;
	};

}

#endif