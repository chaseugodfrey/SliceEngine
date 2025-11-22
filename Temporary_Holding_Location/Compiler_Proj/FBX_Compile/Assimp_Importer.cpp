#include "Assimp_Importer.h"

#include <glm/detail/type_mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

namespace Geometry {

    constexpr uint16_t version_number = 4;
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
        //if (options.contains("assetType") && options["assetType"] == ".anim") {
        //    //default is mdl
        //    mesh = false;
        //}
       
        //set compile options depending on what type it is
        int remove_flags = 0;
       
        remove_flags |= aiComponent_CAMERAS | aiComponent_COLORS | aiComponent_LIGHTS;
        if (options.contains("static") && options["static"]) {
            remove_flags |= aiComponent_ANIMATIONS | aiComponent_BONEWEIGHTS;   //no need to export bone weights
            postprocess_flags |= aiProcess_OptimizeGraph;

            static_mesh = true;
        }
        if (options.contains("smoothing_angle")) {
            float smooth = options["smoothing_angle"];
            importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, smooth);
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

  /*      Model mdl;
        if (!mdl.load_model(src, scene)) {
            return false;
        }
        mdl.SaveModelToFile(dst);*/

        Model mdl;
        if (static_mesh) {
            //simply make the .mdl file
            mdl.load_static_model(src);
            mdl.SaveModelToFile(dst); 
        }
        else {
            //Create .skl file, .animpkg file, .mdl file
            std::filesystem::path dest(dst);

            Skeleton skele;
            skele.load(src);
            dest.replace_extension(".skl");
            skele.Save_Skeleton(dest.string().c_str());

            mdl.load_skinned_model(src, skele);
            dest.replace_extension(".mdl");
            mdl.SaveModelToFile(dest.string().c_str());

            AnimationPackage anim_pkg;
            anim_pkg.Load(src, skele);
            dest.replace_extension(".animpkg");
            anim_pkg.SavePackage(dest.string().c_str());
        }

        return false;
    }

    /*
    * Model
    */

#if !COMPILE_ONLY
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
        M *= glm::translate(glm::mat4(1.f), node.position);
        M *= glm::toMat4(node.rotation);
        M *= glm::scale(glm::mat4(1.f), node.scale);

        glm::mat4 global_transform = parent * M;

        for (auto& it : node.mesh_ref) {
            Mesh const& mesh = meshes[it];
            glBindVertexArray(mesh.vao);

            GLuint uniform = shader.GetUniformLoc("M");
            glUniformMatrix4fv(uniform, 1, false, glm::value_ptr(node.local_tform));

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
    bool Model::load_static_model(const char* file_name) {
        Assimp::Importer importer;
        int remove_flags{};
        remove_flags |= aiComponent_CAMERAS | aiComponent_COLORS | aiComponent_LIGHTS | aiComponent_ANIMATIONS | aiComponent_BONEWEIGHTS | aiComponent_MATERIALS;

        //remove components from the import itself
        importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, remove_flags);
        //remove lines and points - only triangles allowed
        importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);


        const aiScene* scene = importer.ReadFile(file_name, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_RemoveComponent | aiProcess_OptimizeGraph);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cout << "Assimp Error: " << importer.GetErrorString() << std::endl;
            return false;
        }
        init = true;
        is_static = true;
        //Create the meshes
        aiMatrix4x4 identity{};
        process_node(scene->mRootNode, scene, identity, root_node);

        return true;
    }


    bool Model::load_skinned_model(const char* file_name, Skeleton const& skeleton) {
        Assimp::Importer importer;
        int remove_flags{};
        remove_flags |= aiComponent_CAMERAS | aiComponent_COLORS | aiComponent_LIGHTS | aiComponent_MATERIALS;

        //remove components from the import itself
        importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, remove_flags);
        //remove lines and points - only triangles allowed
        importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);


        const aiScene* scene = importer.ReadFile(file_name, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_RemoveComponent);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cout << "Assimp Error: " << importer.GetErrorString() << std::endl;
            return false;
        }
        init = true;
        is_static = false;

        /*
        * for now, since using the same .fbx file to create skeleton, mesh, anim
        * dont worry about targetting mesh and treat the entire tree as the skeleton
        */
        
        //Create the meshes
        aiMatrix4x4 identity{};
        process_skin_node(scene->mRootNode, scene, skeleton, identity, root_node);

        return true;
    }

    void Model::process_node(aiNode* node, aiScene const* scene, aiMatrix4x4 const& p_tform, Node& mdl_node) {
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
        std::vector<VertexBone> vert_bones{};

        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            //process vertice
            Vertex v{};
            VertexBone vb{};

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
        //process indices
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
            auto& face = mesh->mFaces[i];
            //num indices shld always be 3 because we triangulated
            for (unsigned int j = 0; j < face.mNumIndices; ++j) {
                indices.emplace_back(face.mIndices[j]);
            }
        }
        assert(indices.size() % 3 == 0);
       // assert(vert_bones.size() == vertices.size());

        Mesh m = Mesh(std::move(vertices), std::move(indices), std::move(vert_bones));
#if !COMPILE_ONLY
        m.setup_mesh();
#endif
        return m;
    }


    void Model::process_skin_node(aiNode* node, aiScene const* scene, Skeleton const& skeleton, aiMatrix4x4 const& p_tform, Node& mdl_node) {
        aiMatrix4x4 n_tform = p_tform * node->mTransformation;
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
            meshes.emplace_back(process_skin_mesh(m, scene, skeleton));
            meshes.back().name = m->mName.C_Str();
        }
        mdl_node.children.resize(node->mNumChildren);
        for (int i = 0; i < node->mNumChildren; ++i) {
            process_skin_node(node->mChildren[i], scene, skeleton, n_tform, mdl_node.children[i]);
        }
    }

    Mesh Model::process_skin_mesh(aiMesh* mesh, aiScene const* scene, Skeleton const& skeleton) {
        std::vector<Vertex> vertices{};
        std::vector<unsigned int> indices{};
        std::vector<VertexBone> vert_bones{};

        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            //process vertice
            Vertex v{};
            VertexBone vb{};

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
            vert_bones.emplace_back(vb);
        }

        //Extract bone weights
        for (int i{}; i < mesh->mNumBones; ++i) {
            int num_weights = mesh->mBones[i]->mNumWeights;
            auto* weights = mesh->mBones[i]->mWeights;

            int bone_id = -1;

            std::string bone_name = mesh->mBones[i]->mName.C_Str();
            //get the id
            if (skeleton.bone_map.find(bone_name) != skeleton.bone_map.end()) {
                bone_id = skeleton.bone_map.at(bone_name).idx;
            }
            assert(bone_id != -1);

            for (int j{}; j < num_weights; ++j) {
                auto& w = weights[j];
                assert(w.mVertexId < vert_bones.size());

                vert_bones[w.mVertexId].SetVertexBone( bone_id, w.mWeight);
            }
        }
          //process indices
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
            auto& face = mesh->mFaces[i];
            //num indices shld always be 3 because we triangulated
            for (unsigned int j = 0; j < face.mNumIndices; ++j) {
                indices.emplace_back(face.mIndices[j]);
            }
        }
        assert(indices.size() % 3 == 0);
        // assert(vert_bones.size() == vertices.size());

        Mesh m = Mesh(std::move(vertices), std::move(indices), std::move(vert_bones));
#if !COMPILE_ONLY
        m.setup_mesh();
#endif

        return m;
    }

    void VertexBone::SetVertexBone(int id, float weight) {
        for (int i{}; i < MAX_BONE_INFLUENCE; ++i) {
            if (boneIDs[i] < 0) {
                boneIDs[i] = id;
                weights[i] = weight;
                return;
            }
        }
    }
    /*
    void Model::ExtractBoneWeights(std::vector<VertexBone>& vertices, aiMesh* mesh, const aiScene* scene) {
        for (int i{}; i < mesh->mNumBones; ++i) {
            int num_weights = mesh->mBones[i]->mNumWeights;
            auto* weights = mesh->mBones[i]->mWeights;

            int bone_id = -1;

            std::string bone_name = mesh->mBones[i]->mName.C_Str();

            bone_id = i;   
            assert(bone_id != -1);

            for (int j{}; j < num_weights; ++j) {
                auto& w = weights[j];
                assert(w.mVertexId < vertices.size());

                SetVertexBone(vertices[w.mVertexId], bone_id, w.mWeight);
            }
        }
    }
    */

    /*
    * Mesh
    */

#if !COMPILE_ONLY
	void Mesh::setup_mesh() {
        //vbo
        glCreateBuffers(1, &vbo);
        glNamedBufferStorage(vbo, vertices.size() * sizeof(Vertex), vertices.data(), 0);
        CheckGLError();
        //ebo
        glCreateBuffers(1, &ebo);
        glNamedBufferStorage(ebo, indices.size() * sizeof(unsigned int), indices.data(), 0);
        CheckGLError();

        //vao
        glCreateVertexArrays(1, &vao);
        glEnableVertexArrayAttrib(vao, 0);
        glEnableVertexArrayAttrib(vao, 1);
        glEnableVertexArrayAttrib(vao, 2);
        CheckGLError();
        
        glEnableVertexArrayAttrib(vao, 3);
        glEnableVertexArrayAttrib(vao, 4);
        glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, false, offsetof(Vertex, position));
        glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, false, offsetof(Vertex, normal));
        glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, false, offsetof(Vertex, uv));
        CheckGLError();

        glVertexArrayElementBuffer(vao, ebo);
        CheckGLError();

        //Today i learned u can just do this
        glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex));

        glVertexArrayAttribBinding(vao, 0, 0);
        glVertexArrayAttribBinding(vao, 1, 0);
        glVertexArrayAttribBinding(vao, 2, 0);

        if (vert_bones.size()) {
            glCreateBuffers(1, &vbbo);
            glNamedBufferStorage(vbbo, vert_bones.size() * sizeof(VertexBone), vert_bones.data(), 0);
            CheckGLError();
            glVertexArrayAttribIFormat(vao, 3, 4, GL_INT, offsetof(VertexBone, boneIDs));
            glVertexArrayAttribFormat(vao, 4, 4, GL_FLOAT, false, offsetof(VertexBone, weights));
            glVertexArrayVertexBuffer(vao, 1, vbbo, 0, sizeof(VertexBone));
            CheckGLError();
            glVertexArrayAttribBinding(vao, 3, 1);
            glVertexArrayAttribBinding(vao, 4, 1);
        }

        CheckGLError();
        glBindVertexArray(0);
	}

    void Mesh::Release() {
        glDeleteBuffers(1, &ebo);
        glDeleteBuffers(1, &vbbo);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
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
            m.setup_mesh();
        }
        init = true;
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


    void Model::pack_data(char* const buffer, uint64_t& offset) const {
        memcpy(buffer + offset, &is_static, 1); offset += 1;
        uint32_t source{};
        //name
        source = name.size();
        memcpy(buffer + offset, &source, i_size); offset += i_size;
        memcpy(buffer + offset, name.data(), source); offset += source;
        //meshes
        source = meshes.size();
        memcpy(buffer + offset, &source, i_size); offset += i_size;
        if (is_static) {
            for (auto const& m : meshes) {
                m.pack_static_data(buffer, offset);
            }
        }
        else {
            for (auto const& m : meshes) {
                m.pack_skin_data(buffer, offset);
            }
        }

        pack_node_data(buffer, offset, root_node);
    }
    void Model::unpack_data(char const* const buffer, uint64_t& offset) {
        //extract is static to decide
        memcpy(&is_static, buffer + offset, 1); offset += 1;
        uint32_t dest{};
        //name
        memcpy(&dest, buffer + offset, i_size); offset += i_size;
        name.resize(dest);
        memcpy(name.data(), buffer + offset, dest); offset += dest;
        //meshes
        memcpy(&dest, buffer + offset, i_size); offset += i_size;
        meshes.resize(dest);

        if (is_static) {
            for (auto& m : meshes) {
                m.unpack_static_data(buffer, offset);
            }
        }
        else {
            for (auto& m : meshes) {
                m.unpack_skin_data(buffer, offset);
            }
        }

        //node
        unpack_node_data(buffer, offset, root_node);
        return;
    }
    /*
    * because this data is packed recursively,
    * its best to retrieve the data recursively too
    */
    void Model::pack_node_data(char* const buffer, uint64_t& offset, Node const& node) const {
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
    }
    void Model::unpack_node_data(char const* const buffer, uint64_t& offset, Node& node) {
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
    
    uint64_t Model::get_model_size() const {
        uint64_t size{};
        size += 1;                              //is static
        size += i_size + name.size() + i_size;  //name size + name + mesh count

        if (is_static) {
            for (auto const& m : meshes) {
                size += m.get_static_mesh_size();
            }
        }
        else {
            for (auto const& m : meshes) {
                size += m.get_skin_mesh_size();
            }
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
    

    uint64_t Mesh::get_static_mesh_size() const {
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
    uint64_t Mesh::get_skin_mesh_size() const {
        uint64_t size{};

        size = i_size +                         //name size
            name.size() +                       //name
            i_size +                            //vtx size
            i_size +                            //num vtx
            i_size +                            //idx size
            i_size +                            //num idx
            i_size +                            //vb size
            i_size +                            //num vb
            vertices.size() * sizeof(Vertex) +  //vtx
            indices.size() * i_size +           //idx
            vert_bones.size() * sizeof(VertexBone); //vb

        return size;
    }
    void Mesh::unpack_skin_data(char const* const buffer, uint64_t& offset) {
        uint32_t dest{};

        //name
        memcpy(&dest, buffer + offset, i_size); offset += i_size;
        name.resize(dest);
        memcpy(name.data(), buffer + offset, dest); offset += dest;

        uint32_t vert_size{}, idx_size{}, vb_size{};
        uint32_t num_vert{}, num_idx{}, num_vb{};

        //buffer details
        memcpy(&vert_size, buffer + offset, i_size); offset += i_size;
        memcpy(&num_vert, buffer + offset, i_size); offset += i_size;

        memcpy(&idx_size, buffer + offset, i_size); offset += i_size;
        memcpy(&num_idx, buffer + offset, i_size); offset += i_size;

        memcpy(&vb_size, buffer + offset, i_size); offset += i_size;
        memcpy(&num_vb, buffer + offset, i_size); offset += i_size;

        //buffer
        uint64_t vert_buffer_size = (uint64_t)vert_size * num_vert;
        uint64_t idx_buffer_size = (uint64_t)idx_size * num_idx;
        uint64_t vert_bone_buffer_size = (uint64_t)vb_size * num_vb;

        vertices.resize(num_vert);
        indices.resize(num_idx);
        vert_bones.resize(num_vb);
        memcpy(vertices.data(), buffer + offset, vert_buffer_size); offset += vert_buffer_size;
        memcpy(indices.data(), buffer + offset, idx_buffer_size); offset += idx_buffer_size;
        memcpy(vert_bones.data(), buffer + offset, vert_bone_buffer_size); offset += vert_bone_buffer_size;
    }
    void Mesh::pack_skin_data(char* const buffer, uint64_t& offset) const {
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

        source = sizeof(VertexBone);
        memcpy(buffer + offset, &source, i_size); offset += i_size;
        source = vert_bones.size();
        memcpy(buffer + offset, &source, i_size); offset += i_size;

        //buffer
        uint64_t vert_buffer_size = vertices.size() * sizeof(Vertex);
        uint64_t idx_buffer_size = indices.size() * sizeof(unsigned int);
        uint64_t vert_bone_buffer_size = vert_bones.size() * sizeof(VertexBone);

        memcpy(buffer + offset, vertices.data(), vert_buffer_size); offset += vert_buffer_size;
        memcpy(buffer + offset, indices.data(), idx_buffer_size); offset += idx_buffer_size;
        memcpy(buffer + offset, vert_bones.data(), vert_bone_buffer_size); offset += vert_bone_buffer_size;
    }
    void Mesh::pack_static_data(char* const buffer, uint64_t& offset) const {
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

        memcpy(buffer + offset, vertices.data(), vert_buffer_size); offset += vert_buffer_size;
        memcpy(buffer + offset, indices.data(), idx_buffer_size); offset += idx_buffer_size;
    }
    void Mesh::unpack_static_data(char const* const buffer, uint64_t& offset) {
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
}