//#include "Geometry.h"
//
//constexpr auto PI = 3.14159265358f;
//
//namespace Geometry {
//	namespace {
//		bool float_nearEqual(float lhs, float rhs, float eps) {
//			return (lhs > rhs - eps) && (lhs < rhs + eps);
//		}
//
//		void recalculate_normals(Mesh& mesh) {
//			unsigned int num_idx = mesh.raw_indices.size();
//			mesh.normals.reserve(num_idx);
//
//			for (unsigned int i{}; i < num_idx; i += 3) {
//				const int i0 = mesh.raw_indices[i];
//				const int i1 = mesh.raw_indices[i + 1];
//				const int i2 = mesh.raw_indices[i + 2];
//
//				glm::vec3 pos0 = mesh.positions[i0];
//				glm::vec3 pos1 = mesh.positions[i1];
//				glm::vec3 pos2 = mesh.positions[i2];
//
//				glm::vec3 v0 = pos1 - pos0;
//				glm::vec3 v1 = pos2 - pos0;
//
//				glm::vec3 norm = glm::normalize(glm::cross(v0, v1));
//
//				mesh.normals[i] = norm;
//				mesh.normals[i + 1] = norm;
//				mesh.normals[i + 2] = norm;
//			}
//		}
//
//		void process_normals(Mesh& mesh, float smoothing_angle) {
//			const float cos_angle = glm::cos((PI - glm::radians(smoothing_angle)));
//			const bool is_hard_edge{float_nearEqual(smoothing_angle, 180.f, FLT_EPSILON)};	//flag to check if everything is hard edge
//			const bool is_soft_edge{float_nearEqual(smoothing_angle, 0.f, FLT_EPSILON) };	//flag to check if everything is soft edge
//		
//			const unsigned int num_idx = mesh.raw_indices.size();
//			const unsigned int num_pos = mesh.positions.size();
//			assert(num_idx && num_pos);
//
//			mesh.indices.resize(num_idx);
//			std::vector<std::vector<unsigned int>> idx_counter(num_pos);	//keeps track of which indices refer to this position
//
//			for (int i = 0; i < num_idx; ++i) {
//				idx_counter[mesh.raw_indices[i]].emplace_back(i);
//			}
//
//			for (int i{}; i < num_pos; ++i) {
//				//Get the list of indexes that refer to itself
//				auto& idx_ref = idx_counter[i];
//				int num_ref = idx_ref.size();
//				for (int ref{}; ref < num_ref; ++ref) {
//					//Point this index to a vertex(NOT POSITION!!)
//					mesh.indices[idx_ref[ref]] = mesh.vertices.size();
//					Vertex& vertex = mesh.vertices.emplace_back();
//					//use the current raw idx to access the correct position
//					vertex.position = mesh.positions[mesh.raw_indices[idx_ref[ref]]];
//
//					glm::vec3 n1 = mesh.normals[idx_ref[ref]];
//					if (!is_hard_edge) {	//if may not be split
//						//loop through the other indexes that refer to this position
//						for (int pos{ref + 1}; pos < num_ref; ++pos) {
//							float dot{};
//							glm::vec3 n2 = mesh.normals[idx_ref[pos]];
//							if (!is_soft_edge) {	//if may be split
//								//check if within smoothing angle
//								dot = glm::dot(n1, n2) / glm::length(n1);	//n2 is normalized, but n1 might be modified
//								
//							}
//							if (is_soft_edge || dot >= cos_angle) {	//if no split
//								n1 += n2;
//								//set it to be assigned to the same vertex - no split
//								mesh.indices[idx_ref[pos]] = mesh.indices[idx_ref[ref]];
//								//since its using the same vertex, remove it from the ref list so
//								//it dosent create a new vertex in the outer loop
//								idx_ref.erase(idx_ref.begin() + pos);
//								--num_ref;
//								--pos;
//							}
//						}
//					}
//					vertex.normal = glm::normalize(n1);
//				}
//			}
//		}
//
//		void process_uvs(Mesh& mesh) {
//			//if uv arnt the same need to split it
//			std::vector<Vertex> old_vertices{};
//			std::vector<unsigned int> old_indices(mesh.indices.size());
//			//swap the buffers for now, will fill into the ones inside mesh
//			old_vertices.swap(mesh.vertices);
//			old_indices.swap(mesh.indices);
//
//			const unsigned int num_vert = old_vertices.size();
//			const unsigned int num_idx = old_indices.size();
//			assert(num_vert && num_idx);
//
//			//idx counter, same as process normal
//			std::vector<std::vector<unsigned int>> idx_counter(num_vert);	//keeps track of which indices refer to this position
//
//			for (int i = 0; i < num_idx; ++i) {
//				idx_counter[old_indices[i]].emplace_back(i);
//			}
//
//			for (int i{}; i < num_vert; ++i) {
//				auto& refs = idx_counter[i];
//				int num_ref = refs.size();
//				for (int ref{}; ref < num_ref; ++ref) {
//					//same as normals, we create the vertices as we go and when thrs a vert split,
//					//the idx will change
//					mesh.indices[refs[ref]] = mesh.vertices.size();
//					//begin with the original vertex used
//					Vertex& vert = old_vertices[old_indices[refs[ref]]];
//
//					//for now only 1 uv set
//					vert.uv = mesh.uv_sets[0][refs[ref]];
//					mesh.vertices.emplace_back(vert);
//					
//					//Check if need to split by comparing if uv are diff
//					for (int pos{ ref + 1 }; pos < num_ref; ++pos) {
//						glm::vec2& uv = mesh.uv_sets[0][refs[pos]];
//						if (float_nearEqual(vert.uv.x, uv.x, FLT_EPSILON) &&
//							float_nearEqual(vert.uv.y, uv.y, FLT_EPSILON)) {
//							//since uv is the same, just use the same vert, no split
//							mesh.indices[refs[pos]] = mesh.indices[refs[ref]];
//							
//							//same as normals
//							refs.erase(refs.begin() + pos);
//							--num_ref;
//							--pos;
//						}
//					}
//				}
//			}
//		}
//
//		void pack_vertices_static(Mesh& mesh) {
//			int num_vert = mesh.vertices.size();
//			int num_idx = mesh.indices.size();
//			assert(num_vert && num_idx && (num_idx % 3 == 0));
//
//			mesh.packed_vertice_static.reserve(num_vert);
//			for (int i{}; i < num_vert; ++i) {
//				Vertex& vert = mesh.vertices[i];
//				const unsigned char signs = ((vert.normal.z > 0.f) << 1);
//				const unsigned short norm_x = pack_float<16>(vert.normal.x, -1.f, 1.f);
//				const unsigned short norm_y = pack_float<16>(vert.normal.y, -1.f, 1.f);
//
//				mesh.packed_vertice_static.emplace_back(packed_vertex::Vertex_Static{
//					vert.position,
//					{0,0,0}, signs,
//					{norm_x,norm_y}, {},
//					vert.uv
//					});
//			}
//		}
//
//		void process_vertices(Mesh& mesh, Geometry_import_settings const& settings) {
//			assert((mesh.raw_indices.size() % 3) == 0);
//
//			if (settings.calculate_normals || mesh.normals.empty()) {
//				recalculate_normals(mesh);
//			}
//			process_normals(mesh, settings.smoothing_angle);
//
//			if (!mesh.uv_sets.empty()) {
//				process_uvs(mesh);
//			}
//			pack_vertices_static(mesh);
//		}
//
//		void process_scene(Scene& scene, Geometry_import_settings const& settings) {
//			for (auto& lod : scene.lod_groups) {
//				for (auto& mesh : lod.meshes) {
//					process_vertices(mesh, settings);	//create soft/hard edges by splitting vertices
//				}
//			}
//		}
//
//		uint64_t GetMeshSize(Mesh const& mesh) {
//			constexpr uint64_t uint_size = sizeof(unsigned int);
//			constexpr uint64_t float_size = sizeof(float);
//
//			const uint64_t num_vert = mesh.vertices.size();
//			const uint64_t vert_buffer_size = sizeof(packed_vertex::Vertex_Static) * num_vert;
//
//			const uint64_t idx_size = (num_vert < (1u << 16)) ? sizeof(uint16_t) : sizeof(uint32_t);
//			const uint64_t idx_buffer_size = mesh.indices.size() * idx_size;
//
//			uint64_t size{};
//			size += uint_size		//length of name
//				+ mesh.name.size()	//actual name
//				+ uint_size			//mesh id
//				+ uint_size			//vert size
//				+ uint_size			//number of verts
//				+ uint_size			//index size
//				+ uint_size			//number of idx
//				+ float_size		//lod threshold
//				+ vert_buffer_size	//vert buffer
//				+ idx_buffer_size;	//idx buffer
//			return size;
//		}
//
//		uint64_t GetSceneSize(Scene const& scene) {
//			constexpr uint64_t uint_size = sizeof(unsigned int);
//			constexpr uint64_t float_size = sizeof(float);
//
//			uint64_t size{};
//			size += uint_size;			//stores name length
//			size += scene.name.size();	//actual name length
//			size += uint_size;			//stores lod count
//			for (auto& lod : scene.lod_groups) {
//				uint64_t lod_size{};
//				lod_size += uint_size;	//stores name length
//				lod_size += lod.name.size();//actual name length
//				lod_size += uint_size;	//number of meshes
//				for (auto& mesh : lod.meshes) {
//					lod_size += GetMeshSize(mesh);
//				}
//				size += lod_size;
//			}
//			return size;
//		}
//
//		void pack_data(Scene const& scene, Scene_data& data) {
//			auto scene_size = GetSceneSize(scene);
//			//start to actually make the data
//			//data.size = scene_size;
//			//data.data = new unsigned char[scene_size];
//		}
//	}
//
//	bool Fbx_Context::init_fbx() {
//
//
//		return true;
//	}
//	void Fbx_Context::load_fbx_file(const char* file) {
//
//	}
//
//	
//}