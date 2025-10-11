#ifndef GEOMETRY
#define	GEOMETRY
//
//#include <string>
//#include <vector>
//#include <glm/glm.hpp>
//#include <fbxsdk.h>
//
//namespace Geometry {
//	struct Geometry_import_settings {
//		float smoothing_angle{};	//float for defining hard vs soft edge
//		bool calculate_normals{};	//flag to calculate normals instead of using imported values
//		bool calculate_tangents{};	//flag to calculate tangents instead of using imported
//		bool reverse_handedness{};	//flag for if need to convert Left-hand to Right-hand
//		bool import_texture{};		//flag for if textures should be imported from the fbx
//		bool import_animations{};	//flag for if animations should be imported from fbx
//	};
//
//	struct Vertex {
//		glm::vec4 tangent{};
//		glm::vec3 position{};
//		glm::vec3 normal{};
//		glm::vec2 uv{};
//	};
//
//	//possible different kinds of final vertex output
//	namespace packed_vertex {
//		struct Vertex_Static {
//			glm::vec3 position{};
//			unsigned char padding[3];
//			unsigned char t_sign;	//bit 0: tangent handed * tangent z-sign,
//									//bit 1: normal z sign, 0 = -1, 1 = +1
//			unsigned short normal[2];	//stores only x and y values, z is calced using t_sign
//			unsigned short tangent[2];
//			glm::vec2 uv{};
//		};
//	};
//
//	struct Mesh {
//		//Initial data -> loaded from file basically
//		std::vector<glm::vec3>	positions;			//position of each verticein mesh
//		std::vector<glm::vec3>	normals;
//		std::vector<glm::vec4>	tangents;
//		std::vector<std::vector<glm::vec2>>	uv_sets;//multiple uvs possible for 1 mesh
//
//		std::vector<unsigned int>	raw_indices;	//raw indices that refer to raw position indexes
//		//Processed Data
//		std::vector<Vertex>			vertices;
//		std::vector<unsigned int>	indices;
//
//		//Packed Data -> the one actually getting saved to file
//		std::string name;
//		std::vector<packed_vertex::Vertex_Static> packed_vertice_static;
//		float lod_threshold{ -1.f };
//		unsigned int lod_id{ FBXSDK_UINT_MAX };
//	};
//
//	struct Lod_Group {
//		std::string name;
//		std::vector<Mesh> meshes;
//	};
//
//	struct Scene {
//		std::string name;
//		std::vector<Lod_Group> lod_groups;
//	};
//
//	struct Scene_data {
//		unsigned char* data;		//binary blob for objects in the scene - maybe void ptr is btr
//		unsigned int size;			//size of blob
//		Geometry_import_settings settings;
//	};
//
//	class Fbx_Context {
//	public:
//		Fbx_Context(const char* file, Scene* scene, Scene_data* data) :
//		scene(scene), scene_data(data){
//			assert(file && scene && data);
//			if (init_fbx()) {
//				load_fbx_file(file);
//			}
//		}
//
//		~Fbx_Context() {
//			fbx_scene->Destroy();
//			fbx_manager->Destroy();
//		}
//
//		constexpr bool is_valid() const { return fbx_manager && fbx_scene; }
//		constexpr float scale() const { return scene_scale; }
//	private:
//		bool init_fbx();
//		void load_fbx_file(const char* file);
//
//		Scene*		scene{};
//		Scene_data* scene_data{};
//		FbxManager* fbx_manager{};
//		FbxScene*	fbx_scene{};
//		float		scene_scale{1.f};
//	};
//
//	void process_scene(Scene& scene, Geometry_import_settings const& settings);
//	void pack_data(Scene const& scene, Scene_data& data);
//
//	template<unsigned int bits>
//	constexpr unsigned int pack_unit_float(float f) {
//		static_assert(bits <= sizeof(unsigned int) * 8);
//		assert(f >= 0.f && f <= 1.f);
//
//		//calc max value with allocated bits - 1
//		constexpr float chunks = (float)((1u << bits) - 1);
//		return (unsigned int)(chunks * f + 0.5f);	//multiply that max value with float and round to get int
//	}
//
//	template<unsigned int bits>
//	constexpr float unpack_unit_float(unsigned int i) {
//		static_assert(bits <= sizeof(unsigned int) * 8);
//		assert(i < (1u << bits));
//
//		//calc max value with allocated bits - 1
//		constexpr float chunks = (float)((1u << bits) - 1);
//		return (float)i / chunks;	//reverse multiplication from packing
//	}
//
//	template<unsigned int bits>
//	constexpr unsigned int pack_float(float f, float min, float max) {
//		assert(min < max);
//		assert(f <= max && f >= min);
//		const float interp = (f - min) / (max - min);
//		return pack_unit_float<bits>(interp);
//	}
//
//	template<unsigned int bits>
//	constexpr unsigned int unpack_float(unsigned int i, float min, float max) {
//		assert(min < max);
//		return unpack_unit_float<bits>(i) * (max - min) + min;
//	}
//}
//
#endif // ! GEOMETRY
