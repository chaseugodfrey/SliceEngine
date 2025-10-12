#include "Assimp_Importer.h"

#include <glm/detail/type_mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

namespace Geometry {

    constexpr uint16_t version_number = 2;
    constexpr uint64_t i_size = sizeof(unsigned int);
    constexpr uint64_t f_size = sizeof(float);

    namespace {
        glm::mat4 AssimpMatToGLM(aiMatrix4x4 const& mat) {
            glm::mat4 glm_mat = {
                {mat.a1,mat.b1,mat.c1,mat.d1},
                {mat.a2,mat.b2,mat.c2,mat.d2},
                {mat.a3,mat.b3,mat.c3,mat.d3},
                {mat.a4,mat.b4,mat.c4,mat.d4}
            };
            //glm_mat = glm::transpose(glm_mat);
            return glm_mat;
        }
    }

    /*
    * Importer
    */

    void FBX_Compiler::set_options(nlohmann::json const& options, Assimp::Importer& importer) {
        //Check what type ok compiling it is
        //only mesh or animation for now
        if (options.contains("assetType") && options["assetType"] == ".anim") {
            //default is mdl
            mesh = false;
        }
       
        //set compile options depending on what type it is
        int remove_flags = 0;
        if (mesh) { //set mesh compiler options
            remove_flags |= aiComponent_CAMERAS | aiComponent_COLORS | aiComponent_LIGHTS | aiComponent_ANIMATIONS | aiComponent_BONEWEIGHTS;
            if (options.contains("static") && options["static"]) {
                postprocess_flags |= aiProcess_OptimizeGraph;
            }
            if (options.contains("smoothing_angle")) {
                float smooth = options["smoothing_angle"];
                importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, smooth);
            }
        }
        else {
            //ill deal with this later
        }
        //remove components from the import itself
        importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, remove_flags);
        //remove lines and points - only triangles allowed
        importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
        postprocess_flags |= aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_RemoveComponent;
        //postprocess_flags = aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_RemoveRedundantMaterials | aiProcess_RemoveComponent;
    }

    bool FBX_Compiler::Compile_Asset(const char* src, const char* dst, nlohmann::json const& options) {
        if (!std::filesystem::exists(src)) {
            std::cout << "Invalid src" << std::endl;
            return false;
        }
        Assimp::Importer importer;
        set_options(options, importer);

        const aiScene* scene = importer.ReadFile(src, postprocess_flags);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cout << "Assimp Error: " << importer.GetErrorString() << std::endl;
            return false;
        }

        Model mdl;
        if (!mdl.load_model(src, scene)) {
            return false;
        }
        mdl.SaveModelToFile(dst);
    }

    void FBX_Compiler::Compile_Primitive(Primitive_Type type, const char* dst) {
        Model mdl;
        switch (type) {
        case Cube:
            mdl.Create_Cube();
            break;
        }
        mdl.SaveModelToFile(dst);
    }

    /*
    * Model
    */

#if !COMPILE_ONLY
    void Model::Init(const char* f) {
        Assimp::Importer importer;
        int remove_flags{};
        remove_flags |= aiComponent_CAMERAS | aiComponent_COLORS | aiComponent_LIGHTS | aiComponent_ANIMATIONS | aiComponent_BONEWEIGHTS;

        //remove components from the import itself
        importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, remove_flags);
        //remove lines and points - only triangles allowed
        importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);


        const aiScene* scene = importer.ReadFile(f, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_RemoveComponent);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cout << "Assimp Error: " << importer.GetErrorString() << std::endl;
            return;
        }
        load_model(f, scene);
        init = true;
    }
    void Model::Release() {
        if (!init) {
            return;
        }
        init = false;
        for (Mesh& m : meshes) {
            m.Release();
        }
    }
    
    void Model::draw_node(GL_Shader& shader, Node& node, glm::mat4 const& parent) {
        glm::mat4x4 M(1.f);
        M = glm::translate(M, node.position);
        M *= glm::mat4_cast(node.rotation);
        M = glm::scale(M, node.scale);

        glm::mat4 global_transform = parent * M;

        for (auto& it : node.mesh_ref) {
            Mesh const& mesh = meshes[it];
            glBindVertexArray(mesh.vao);

            GLuint uniform = shader.GetUniformLoc("M");
            glUniformMatrix4fv(uniform, 1, false, glm::value_ptr(global_transform));

            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);

            glBindVertexArray(0);
        }

        for (auto& c : node.children) {
            draw_node(shader, c, global_transform);
        }
    }

    void Model::Draw(GL_Shader& shader) {
        if (!init) {
            return;
        }
        draw_node(shader, root_node, glm::identity<glm::mat4>());
        return;
    }
#endif
    bool Model::load_model(const char* file_name, const aiScene* scene) {

        //Create the meshes
        aiMatrix4x4 identity{};
        process_node(scene->mRootNode, scene, identity, root_node);

#if !COMPILE_ONLY
        int num_mat = scene->mNumMaterials;
        std::cout << "Total materials: " << num_mat << std::endl;
        //for (int i = 0; i < num_mat; ++i) {
        //    auto* material = scene->mMaterials[1];
        //    std::cout << "\tMaterial " << i << ": " << std::endl;
        //    int num_prop = material->mNumProperties;
        //    int num_alloc = material->mNumAllocated;
        //    std::cout << "\t\tNum Properties: " << num_prop << std::endl;
        //    std::cout << "\t\tNum Allocated: " << num_alloc << std::endl;
        //    for (int p{}; p < num_prop; ++p) {
        //        auto* prop = material->mProperties[p];
        //        std::cout << "\t\tkey" << prop->mKey.C_Str() << std::endl;
        //    }
        //}
#endif
        return true;
    }


    void Model::Create_Cube() {
        name = "Cube";
        //Create a single mesh
        meshes.resize(1);
        auto& mesh = meshes.back();
        root_node.mesh_ref.resize(1);
        root_node.mesh_ref[0] = 0;
        //each vertex needs to be duplicated 3 times because of normals
        /*
        * width of cube is 1,current uv coord will be 0-1 on each face for now
        * will probably need to change it to a cube map for cubes specifically (maybe not needed)
        */
        auto& vertices = mesh.vertices;
        vertices.clear();
        //----------bot
        //left-bot-back     0-2
        vertices.emplace_back(Vertex{ {-0.5f,-0.5f,-0.5f},{-1.f,0.f,0.f},{0.f,1.f} });
        vertices.emplace_back(Vertex{ {-0.5f,-0.5f,-0.5f},{0.f,-1.f,0.f},{0.f,1.f} });
        vertices.emplace_back(Vertex{ {-0.5f,-0.5f,-0.5f},{0.f,0.f,-1.f},{1.f,1.f} });

        //right-bot-back    3-5
        vertices.emplace_back(Vertex{ {0.5f,-0.5f,-0.5f},{1.f,0.f,0.f},{1.f,1.f} });
        vertices.emplace_back(Vertex{ {0.5f,-0.5f,-0.5f},{0.f,-1.f,0.f},{1.f,1.f} });
        vertices.emplace_back(Vertex{ {0.5f,-0.5f,-0.5f},{0.f,0.f,-1.f},{0.f,1.f} });

        //left-bot-front    6-8
        vertices.emplace_back(Vertex{ {-0.5f,-0.5f,0.5f},{-1.f,0.f,0.f},{1.f,1.f} });
        vertices.emplace_back(Vertex{ {-0.5f,-0.5f,0.5f},{0.f,-1.f,0.f},{0.f,0.f} });
        vertices.emplace_back(Vertex{ {-0.5f,-0.5f,0.5f},{0.f,0.f,1.f},{0.f,1.f} });

        //right-bot-front   9-11
        vertices.emplace_back(Vertex{ {0.5f,-0.5f,0.5f},{1.f,0.f,0.f},{0.f,1.f} });
        vertices.emplace_back(Vertex{ {0.5f,-0.5f,0.5f},{0.f,-1.f,0.f},{1.f,0.f} });
        vertices.emplace_back(Vertex{ {0.5f,-0.5f,0.5f},{0.f,0.f,1.f},{1.f,1.f} });

        //----------top
        //left-top-back     12-14
        vertices.emplace_back(Vertex{ {-0.5f,0.5f,-0.5f},{-1.f,0.f,0.f},{0.f,0.f} });
        vertices.emplace_back(Vertex{ {-0.5f,0.5f,-0.5f},{0.f,1.f,0.f},{0.f,0.f} });
        vertices.emplace_back(Vertex{ {-0.5f,0.5f,-0.5f},{0.f,0.f,-1.f},{1.f,0.f} });

        //right-top-back    15-17
        vertices.emplace_back(Vertex{ {0.5f,0.5f,-0.5f},{1.f,0.f,0.f},{1.f,0.f} });
        vertices.emplace_back(Vertex{ {0.5f,0.5f,-0.5f},{0.f,1.f,0.f},{1.f,0.f} });
        vertices.emplace_back(Vertex{ {0.5f,0.5f,-0.5f},{0.f,0.f,-1.f},{0.f,0.f} });

        //left-top-front    18-20
        vertices.emplace_back(Vertex{ {-0.5f,0.5f,0.5f},{-1.f,0.f,0.f},{1.f,0.f} });
        vertices.emplace_back(Vertex{ {-0.5f,0.5f,0.5f},{0.f,1.f,0.f},{0.f,1.f} });
        vertices.emplace_back(Vertex{ {-0.5f,0.5f,0.5f},{0.f,0.f,1.f},{0.f,0.f} });

        //right-top-front   21-23
        vertices.emplace_back(Vertex{ {0.5f,0.5f,0.5f},{1.f,0.f,0.f},{0.f,0.f} });
        vertices.emplace_back(Vertex{ {0.5f,0.5f,0.5f},{0.f,1.f,0.f},{1.f,1.f} });
        vertices.emplace_back(Vertex{ {0.5f,0.5f,0.5f},{0.f,0.f,1.f},{1.f,0.f} });

        auto& indices = mesh.indices;
        indices.clear();
        /*
            front, left, right, top, bot, back
        */
        //front
        indices.emplace_back(20); indices.emplace_back(8); indices.emplace_back(11);
        indices.emplace_back(11); indices.emplace_back(23); indices.emplace_back(20);

        //left
        indices.emplace_back(12); indices.emplace_back(0); indices.emplace_back(6);
        indices.emplace_back(6); indices.emplace_back(18); indices.emplace_back(12);

        //right
        indices.emplace_back(21); indices.emplace_back(9); indices.emplace_back(3);
        indices.emplace_back(3); indices.emplace_back(15); indices.emplace_back(21);

        //top
        indices.emplace_back(13); indices.emplace_back(19); indices.emplace_back(22);
        indices.emplace_back(22); indices.emplace_back(16); indices.emplace_back(13);

        //bot
        indices.emplace_back(7); indices.emplace_back(1); indices.emplace_back(4);
        indices.emplace_back(4); indices.emplace_back(10); indices.emplace_back(7);

        //back
        indices.emplace_back(17); indices.emplace_back(5); indices.emplace_back(2);
        indices.emplace_back(2); indices.emplace_back(14); indices.emplace_back(17);
    }
    void Model::Create_Sphere() {

    }
    void Model::Create_Plane() {

    }

    void Model::process_node(aiNode* node, aiScene const* scene, aiMatrix4x4 p_tform, Node& mdl_node) {
        aiMatrix4x4 n_tform =  p_tform * node->mTransformation;
        glm::mat4 glm_tform = AssimpMatToGLM(n_tform);

        glm::mat4 local_tform = AssimpMatToGLM(node->mTransformation);
        //decompose to rotation and scale and pos
        glm::vec3 pos{}, scale{}, skew{};
        glm::vec4 persp{};
        glm::quat rot{};
        glm::decompose(local_tform, scale, rot, pos, skew, persp);
        mdl_node.position = pos;
        mdl_node.rotation = rot;
        mdl_node.scale = scale;
        
        mdl_node.mesh_ref.resize(node->mNumMeshes);
        mdl_node.name = node->mName.C_Str();
        for (int i = 0; i < node->mNumMeshes; ++i) {
            auto* m = scene->mMeshes[node->mMeshes[i]];
            mdl_node.mesh_ref[i] = meshes.size();
            meshes.emplace_back(process_mesh(m, scene));
            meshes.back().name = m->mName.C_Str();

        }
        mdl_node.children.resize(node->mNumChildren);
        for (int i = 0; i < node->mNumChildren; ++i) {
            process_node(node->mChildren[i], scene, n_tform, mdl_node.children[i]);
        }
    }

    Mesh Model::process_mesh(aiMesh* mesh, aiScene const* scene) {
        std::vector<Vertex> vertices{};
        std::vector<unsigned int> indices{};
        std::vector<Texture> textures{};

        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            //process vertice
            Vertex v{};
#if !COMPILE_ONLY
            v.SetBonesDefault();
#endif

            auto& pos = mesh->mVertices[i];
            v.position = { pos.x, pos.y, pos.z };
            auto& norm = mesh->mNormals[i];
            v.normal = { norm.x, norm.y, norm.z };

            if (/*mesh->HasTextureCoords(i) && */mesh->mTextureCoords[0]) {
                auto& uv = mesh->mTextureCoords[0][i];
                v.uv = { uv.x, uv.y };
            }
            else {
                v.uv = {};
            }

            vertices.emplace_back(v);
        }
#if !COMPILE_ONLY
        ExtractBoneWeights(vertices, mesh, scene);
#endif

        //process indices
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
            auto& face = mesh->mFaces[i];
            //num indices shld always be 3 because we triangulated
            for (unsigned int j = 0; j < face.mNumIndices; ++j) {
                indices.emplace_back(face.mIndices[j]);
            }
        }
        assert(indices.size() % 3 == 0);

        //process materials
        /*
        * Note: i think that this should be linked somehow to the editor's assetmanager because
        * if textures are all exported eventually to a .dds file
        * perhaps textures be exported seperately and not while embedded
        * maybe the material should be simply added into the resource in the form of required textures and their name
        * probably using the below property stuff
        */
        /*
        auto* material = scene->mMaterials[mesh->mMaterialIndex];
        auto& pro = material->mProperties[0];
        pro->mKey;  //the property's key(name)
        pro->mData; //the property's data
        pro->mIndex;//the property's texture index
        */


        return Mesh(std::move(vertices), std::move(indices), std::move(textures));
    }

#if !COMPILE_ONLY

    void Model::SetVertexBone(Vertex& v, int id, float weight) {
        //float smallestWeight = FLT_MAX;
        //int smallestIdx = -1;
        //for (int i{}; i < MAX_BONE_INFLUENCE; ++i) {
        //    if (v.boneIDs[i] < 0) {
        //        v.boneIDs[i] = id;
        //        v.weights[i] = weight;
        //        return;
        //    }
        //    if (v.weights[i] < smallestWeight) {
        //        smallestWeight = v.weights[i];
        //        smallestIdx = i;
        //    }
        //}
        ////Full, replace the smallest
        //if (weight < smallestWeight && smallestIdx != -1) {
        //    v.boneIDs[smallestIdx] = id;
        //    v.weights[smallestIdx] = weight;
        //}
    }
    void Model::ExtractBoneWeights(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene) {
        return;
        for (int i{}; i < mesh->mNumBones; ++i) {
            int num_weights = mesh->mBones[i]->mNumWeights;
            auto* weights = mesh->mBones[i]->mWeights;

            int bone_id = -1;

            std::string bone_name = mesh->mBones[i]->mName.C_Str();
            if (bone_map.find(bone_name) == bone_map.end()) {
                BoneInfo boneInfo;
                boneInfo.id = bone_map.size();
                boneInfo.offset = AssimpMatToGLM(mesh->mBones[i]->mOffsetMatrix);

                bone_id = boneInfo.id;
                bone_map[bone_name] = boneInfo;
            }
            else {
                bone_id = bone_map[bone_name].id;
            }
            assert(bone_id != -1);

            for (int j{}; j < num_weights; ++j) {
                auto& w = weights[j];
                assert(w.mVertexId < vertices.size());

                SetVertexBone(vertices[w.mVertexId], bone_id, w.mWeight);
            }
        }
        //for (auto const& v : vertices) {
        //    if ((v.boneIDs[0] == -1 && v.boneIDs[1] == -1 
        //        && v.boneIDs[2] == -1 && v.boneIDs[3] == -1)
        //        ) {
        //        //std::cout << "vert missing bone id" << std::endl;
        //    }
        //    //std::cout << "vert boneid: " << v.boneIDs[0] << " " << v.boneIDs[1] << " " << v.boneIDs[2] << " " << v.boneIDs[3] << std::endl;
        //}
    }
#endif
    /*
    * Mesh
    */

#if !COMPILE_ONLY
	void Mesh::setup_mesh() {
        //vbo
        glCreateBuffers(1, &vbo);
        glNamedBufferStorage(vbo, vertices.size() * sizeof(Vertex), vertices.data(), 0);

        //ebo
        glCreateBuffers(1, &ebo);
        glNamedBufferStorage(ebo, indices.size() * sizeof(unsigned int), indices.data(), 0);

        //vao
        glCreateVertexArrays(1, &vao);
        glEnableVertexArrayAttrib(vao, 0);
        glEnableVertexArrayAttrib(vao, 1);
        glEnableVertexArrayAttrib(vao, 2);
      //  glEnableVertexArrayAttrib(vao, 3);
      //  glEnableVertexArrayAttrib(vao, 4);
        glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, false, offsetof(Vertex, position));
        glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, false, offsetof(Vertex, normal));
        glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, false, offsetof(Vertex, uv));
#if !COMPILE_ONLY
        //glVertexArrayAttribIFormat(vao, 3, 4, GL_INT, offsetof(Vertex, boneIDs));
        //glVertexArrayAttribFormat(vao, 4, 4, GL_FLOAT, false, offsetof(Vertex, weights));
#endif
        glVertexArrayElementBuffer(vao, ebo);

        //Today i learned u can just do this
        glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex));
       /* glVertexArrayVertexBuffer(vao, 0, vbo, offsetof(Vertex, position), sizeof(Vertex));
        glVertexArrayVertexBuffer(vao, 1, vbo, offsetof(Vertex, normal), sizeof(Vertex));
        glVertexArrayVertexBuffer(vao, 2, vbo, offsetof(Vertex, uv), sizeof(Vertex));
        glVertexArrayVertexBuffer(vao, 3, vbo, offsetof(Vertex, boneIDs), sizeof(Vertex));
        glVertexArrayVertexBuffer(vao, 4, vbo, offsetof(Vertex, weights), sizeof(Vertex));*/


        glVertexArrayAttribBinding(vao, 0, 0);
        glVertexArrayAttribBinding(vao, 1, 0);
        glVertexArrayAttribBinding(vao, 2, 0);
#if !COMPILE_ONLY
       // glVertexArrayAttribBinding(vao, 3, 0);
       // glVertexArrayAttribBinding(vao, 4, 0);
#endif

        glBindVertexArray(0);
	}

    void Mesh::Release() {
        glDeleteBuffers(1, &ebo);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }


    void Vertex::SetBonesDefault() {
       /* for (int i{}; i < MAX_BONE_INFLUENCE; ++i) {
            boneIDs[i] = -1;
            weights[i] = 0.f;
        }*/
    }

#endif
    //Serialization
    bool Model::LoadModelFromFile(const char* file) {
        if (!std::filesystem::exists(file)) {
            return false;
        } 
        //write binary blob to file
        std::fstream fs(file, std::ios::binary | std::ios::in);

        if (!fs.good()) {
            std::cout << "Unable to open file: " << file << std::endl;
            return false;
        }

        constexpr uint64_t header_size = 3 + sizeof(version_number) + sizeof(uint64_t);
        char header_buffer[header_size]{};
        try {
            fs.read(header_buffer, header_size);
        }
        catch (...){
            std::cout << "Error reading file: " << file << std::endl;
            fs.close();
            return false;
        }

        if (fs.fail() || fs.eof()) {
            std::cout << "Unknown file format: " << file << std::endl;
            fs.close();
            return false;
        }

        if (header_buffer[0] != 'M' || header_buffer[1] != 'D' || header_buffer[2] != 'L') {
            std::cout << "Not a proper mdl file: " << file << std::endl;
            fs.close();
            return false;
        }
        auto vers = version_number;
        vers = *((decltype(version_number)*)(header_buffer + 3));
        if (vers != version_number) {
            std::cout << "Wrong version, please recompile: " << file << std::endl;
            fs.close();
            return false;
        }

        //finally, get the file size
        uint64_t buffer_size = *((uint64_t*)(header_buffer + 3 + sizeof(version_number)));

        char* const buffer = new char[buffer_size];
        uint64_t offset{};

        fs.read(buffer, buffer_size);

       /* if (fs.fail()) {
            std::cout << "Something wrong with reading mdl file: " << file << std::endl;
            delete[] buffer;
            return false;
        }*/
        fs.close();

        unpack_data(buffer, offset);

        delete[] buffer;

        return true;
    }

#if !COMPILE_ONLY
    void Model::InitLoadedModel() {
        for (auto& m : meshes) {
            m.Init();
        }
        init = true;
    }
    void Mesh::Init() {
        setup_mesh();
    }
#endif

    void Model::SaveModelToFile(const char* file) {
        std::filesystem::path file_path(file);
        std::filesystem::create_directories(file_path.parent_path());

        constexpr uint64_t header_size = 3 + sizeof(version_number) + sizeof(uint64_t);

        //convert to binary blob first
        uint64_t buffer_size = get_model_size();
        char* const buffer = new char[buffer_size + header_size];
        uint64_t offset{};
        //write header first
        *(buffer + offset) = 'M'; offset += 1;
        *(buffer + offset) = 'D'; offset += 1;
        *(buffer + offset) = 'L'; offset += 1;
        memcpy(buffer + offset, &version_number, sizeof(version_number)); offset += sizeof(version_number);
        memcpy(buffer + offset, &buffer_size, sizeof(uint64_t)); offset += sizeof(uint64_t);

        //write payload
        pack_data(buffer, offset);

        //write binary blob to file
        std::fstream fs(file, std::ios::binary | std::ios::out);
        if (!fs.good()) {
            delete[] buffer;
            return;
        }

        fs.write(buffer, buffer_size + header_size);
        fs.close();

        delete[] buffer;
    }

    void Model::pack_data(char* const buffer, uint64_t& offset) {
        uint32_t source{};
        //name
        source = name.size();
        memcpy(buffer + offset, &source, i_size); offset += i_size;
        memcpy(buffer + offset, name.data(), source); offset += source;
        //meshes
        source = meshes.size();
        memcpy(buffer + offset, &source, i_size); offset += i_size;
        for (auto& m : meshes) {
            m.pack_data(buffer, offset);
        }
        //hierachy
        //Node test;
        //test.local_transform = glm::identity<glm::mat4>();
        //test.mesh_ref.push_back(2);
        //test.mesh_ref.push_back(3);
        //test.mesh_ref.push_back(4);
        //test.children.push_back(Node());
        //test.children.back().local_transform = glm::translate(glm::identity<glm::mat4>(), { -5.f,1.f,2.f });
        //test.children.back().mesh_ref.push_back(3);
        //test.children.back().mesh_ref.push_back(4);
        //test.children.back().mesh_ref.push_back(5);

        //test.children.push_back(Node());
        //test.children.back().local_transform = glm::translate(glm::identity<glm::mat4>(), { 5.f,-1.f,-2.f });
        //test.children.back().mesh_ref.push_back(4);
        //test.children.back().mesh_ref.push_back(5);
        //test.children.back().mesh_ref.push_back(6);

        //pack_node_data(buffer, offset, test);
        pack_node_data(buffer, offset, root_node);
        //uint32_t dest{};
        //memcpy(&dest, buffer + offset - i_size, i_size);
        //assert(dest == 1001);
    }

    /*
    * because this data is packed recursively,
    * its best to retrieve the data recursively too
    */
    void Model::pack_node_data(char* const buffer, uint64_t& offset, Node const& node) {
        uint32_t source{};
        //name
        source = node.name.size();
        memcpy(buffer + offset, &source, i_size); offset += i_size;
        memcpy(buffer + offset, node.name.data(), source); offset += source;
        //mesh refs
        source = node.mesh_ref.size();
        memcpy(buffer + offset, &source, i_size); offset += i_size;
        source *= sizeof(unsigned short);
        memcpy(buffer + offset, node.mesh_ref.data(), source); offset += source;

        //transform
        //memcpy(buffer + offset, glm::value_ptr(node.local_transform), sizeof(glm::mat4));
        //offset += sizeof(glm::mat4);
        memcpy(buffer + offset, glm::value_ptr(node.position), sizeof(glm::vec3)); offset += sizeof(glm::vec3);
        memcpy(buffer + offset, glm::value_ptr(node.rotation), sizeof(glm::quat)); offset += sizeof(glm::quat);
        memcpy(buffer + offset, glm::value_ptr(node.scale), sizeof(glm::vec3)); offset += sizeof(glm::vec3);
        
        
        //children
        source = node.children.size();
        memcpy(buffer + offset, &source, i_size); offset += i_size;
        for (auto const& child : node.children) {
            pack_node_data(buffer, offset, child);
        }
        //source = 1001;    //sanity check
        //memcpy(buffer + offset, &source, i_size); offset += i_size;
    }

    void Model::unpack_node_data(char* const buffer, uint64_t& offset, Node& node) {
        uint32_t dest{};
        //name
        memcpy(&dest, buffer + offset, i_size); offset += i_size;
        name.resize(dest);
        memcpy(name.data(), buffer + offset, dest); offset += dest;
        //mesh refs
        memcpy(&dest, buffer + offset, i_size); offset += i_size;
        node.mesh_ref.resize(dest);
        dest *= sizeof(unsigned short);
        memcpy(node.mesh_ref.data(), buffer + offset, dest); offset += dest;

        //transform
        //memcpy(glm::value_ptr(node.local_transform), buffer + offset, sizeof(glm::mat4));
        //offset += sizeof(glm::mat4);
        memcpy(glm::value_ptr(node.position), buffer + offset, sizeof(glm::vec3)); offset += sizeof(glm::vec3);
        memcpy(glm::value_ptr(node.rotation), buffer + offset, sizeof(glm::quat)); offset += sizeof(glm::quat);
        memcpy(glm::value_ptr(node.scale), buffer + offset, sizeof(glm::vec3)); offset += sizeof(glm::vec3);


        //children
        memcpy(&dest, buffer + offset, i_size); offset += i_size;
        node.children.resize(dest);
        for (auto& child : node.children) {
            unpack_node_data(buffer, offset, child);
        }
        //memcpy(&dest, buffer + offset, i_size); offset += i_size;
        //assert(dest == 1001);   //sanity check - its not sanitying
    }

    void Model::unpack_data(char* const buffer, uint64_t& offset) {
        uint32_t dest{};
        //name
        memcpy(&dest, buffer + offset, i_size); offset += i_size;
        name.resize(dest);
        memcpy(name.data(), buffer + offset, dest); offset += dest;
        //meshes
        memcpy(&dest, buffer + offset, i_size); offset += i_size;
        meshes.resize(dest);
        for (auto& m : meshes) {
            m.unpack_data(buffer, offset);
        }
        //node
        unpack_node_data(buffer, offset, root_node);
        return;
    }

    void Mesh::pack_data(char* const buffer, uint64_t& offset) {
        uint32_t source{};
        //name
        source = name.size();
        memcpy(buffer + offset, &source, i_size); offset += i_size;
        memcpy(buffer + offset, name.data(), source); offset += source;

        //buffer details
        source = sizeof(Vertex);
        memcpy(buffer + offset, &source, i_size); offset += i_size;
        source = vertices.size();
        memcpy(buffer + offset, &source, i_size); offset += i_size;

        source = sizeof(unsigned int);
        memcpy(buffer + offset, &source, i_size); offset += i_size;
        source = indices.size();
        memcpy(buffer + offset, &source, i_size); offset += i_size;

        //buffer
        uint64_t vert_buffer_size = vertices.size() * sizeof(Vertex);
        uint64_t idx_buffer_size = indices.size() * sizeof(unsigned int);

        //note that in this version, using new_vertices.data instead of vertices.data
        memcpy(buffer + offset, vertices.data(), vert_buffer_size); offset += vert_buffer_size;
        memcpy(buffer + offset, indices.data(), idx_buffer_size); offset += idx_buffer_size;
    }

    void Mesh::unpack_data(char* const buffer, uint64_t& offset) {
        uint32_t dest{};

        //name
        memcpy(&dest, buffer + offset, i_size); offset += i_size;
        name.resize(dest);
        memcpy(name.data(), buffer + offset, dest); offset += dest;

        uint32_t vert_size{}, idx_size{};
        uint32_t num_vert{}, num_idx{};

        //buffer details
        memcpy(&vert_size, buffer + offset, i_size); offset += i_size;
        memcpy(&num_vert, buffer + offset, i_size); offset += i_size;

        memcpy(&idx_size, buffer + offset, i_size); offset += i_size;
        memcpy(&num_idx, buffer + offset, i_size); offset += i_size;

        //buffer
        uint64_t vert_buffer_size = vert_size * num_vert;
        uint64_t idx_buffer_size = idx_size * num_idx;

        vertices.resize(num_vert);
        indices.resize(num_idx);
        memcpy(vertices.data(), buffer + offset, vert_buffer_size); offset += vert_buffer_size;
        memcpy(indices.data(), buffer + offset, idx_buffer_size); offset += idx_buffer_size;
    }

    uint64_t Model::get_model_size() const {
        uint64_t size{};

        size = i_size + name.size() + i_size;    //name size + name + mesh count

        for (auto& m : meshes) {
            size += m.get_mesh_size();
        }
        size += get_node_size(root_node);
        return size;
    }

    uint64_t Model::get_node_size(Node const& node) const {
        uint64_t size{};

        size += i_size +        //name size
            node.name.size() +  //name
            i_size +            //mesh ref cnt
            node.mesh_ref.size() * sizeof(unsigned short) + //mesh ref
            i_size;             //children cnt

        //size += sizeof(glm::mat4);  //local tform
        //pos, rot, scale
        size += sizeof(glm::vec3) + sizeof(glm::quat) + sizeof(glm::vec3);
        
        for (auto const& n : node.children) {
            size += get_node_size(n);   //children
        }

        return size;
    }

    uint64_t Mesh::get_mesh_size() const {
        uint64_t size{};

        size = i_size +                         //name size
            name.size() +                       //name
            i_size +                            //vtx size
            i_size +                            //num vtx
            i_size +                            //idx size
            i_size +                            //num idx
            vertices.size() * sizeof(Vertex) +  //vtx
            indices.size() * i_size;            //idx

        return size;
    }
}