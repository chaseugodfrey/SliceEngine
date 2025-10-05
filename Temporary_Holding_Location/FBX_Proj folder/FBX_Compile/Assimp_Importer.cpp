#include "Assimp_Importer.h"

#include <glm/detail/type_mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Geometry {

    constexpr uint16_t version_number = 1;
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
    bool Mesh_Compiler::Compile_Asset(const char* src, const char* dst) {
        Model mdl;
        if (!mdl.load_model(src)) {
            return false;
        }
        mdl.SaveModelToFile(dst);
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

    void Model::Draw(GL_Shader& shader) {
        if (!init) {
            return;
        }
        for (int i = 0; i < meshes.size(); ++i) {
            meshes[i].Draw(shader);
        }
    }
#endif
    bool Model::load_model(const char* file_name) {
        if (!std::filesystem::exists(file_name)) {
            std::cout << "Invalid src" << std::endl;
            return false;
        }
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(file_name, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cout << "Assimp Error: " << importer.GetErrorString() << std::endl;
            return false;
        }

        //Create the meshes
        aiMatrix4x4 identity{};
        process_node(scene->mRootNode, scene, identity);

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

    void Model::process_node(aiNode* node, aiScene const* scene, aiMatrix4x4 p_tform) {
        aiMatrix4x4 n_tform =  p_tform * node->mTransformation;
        glm::mat4 glm_tform = AssimpMatToGLM(n_tform);

        for (int i = 0; i < node->mNumMeshes; ++i) {
            auto* m = scene->mMeshes[node->mMeshes[i]];
            meshes.emplace_back(process_mesh(m, scene));
            meshes.back().transform = AssimpMatToGLM(node->mTransformation);
            meshes.back().name = m->mName.C_Str();
        }
        for (int i = 0; i < node->mNumChildren; ++i) {
            process_node(node->mChildren[i], scene, n_tform);
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
        auto* material = scene->mMaterials[mesh->mMaterialIndex];
        /*
        * Note: i think that this should be linked somehow to the editor's assetmanager because
        * if textures are all exported eventually to a .dds file
        * perhaps textures be exported seperately and not while embedded
        * maybe the material should be simply added into the resource in the form of required textures and their name
        * probably using the below property stuff
        */
        /*
        auto& pro = material->mProperties[0];
        pro->mKey;  //the property's key(name)
        pro->mData; //the property's data
        pro->mIndex;//the property's texture index
        */


        return Mesh(std::move(vertices), std::move(indices), std::move(textures));
    }

#if !COMPILE_ONLY

    void Model::SetVertexBone(Vertex& v, int id, float weight) {
        float smallestWeight = FLT_MAX;
        int smallestIdx = -1;
        for (int i{}; i < MAX_BONE_INFLUENCE; ++i) {
            if (v.boneIDs[i] < 0) {
                v.boneIDs[i] = id;
                v.weights[i] = weight;
                return;
            }
            if (v.weights[i] < smallestWeight) {
                smallestWeight = v.weights[i];
                smallestIdx = i;
            }
        }
        //Full, replace the smallest
        if (weight < smallestWeight && smallestIdx != -1) {
            v.boneIDs[smallestIdx] = id;
            v.weights[smallestIdx] = weight;
        }
    }
    void Model::ExtractBoneWeights(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene) {
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
        for (auto const& v : vertices) {
            if ((v.boneIDs[0] == -1 && v.boneIDs[1] == -1 
                && v.boneIDs[2] == -1 && v.boneIDs[3] == -1)
                ) {
                //std::cout << "vert missing bone id" << std::endl;
            }
            //std::cout << "vert boneid: " << v.boneIDs[0] << " " << v.boneIDs[1] << " " << v.boneIDs[2] << " " << v.boneIDs[3] << std::endl;
        }
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
        glEnableVertexArrayAttrib(vao, 3);
        glEnableVertexArrayAttrib(vao, 4);
        glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, false, offsetof(Vertex, position));
        glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, false, offsetof(Vertex, normal));
        glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, false, offsetof(Vertex, uv));
        glVertexArrayAttribIFormat(vao, 3, 4, GL_INT, offsetof(Vertex, boneIDs));
        glVertexArrayAttribFormat(vao, 4, 4, GL_FLOAT, false, offsetof(Vertex, weights));

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
        glVertexArrayAttribBinding(vao, 3, 0);
        glVertexArrayAttribBinding(vao, 4, 0);

        glBindVertexArray(0);
	}

    void Mesh::Release() {
        glDeleteBuffers(1, &ebo);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }

    void Mesh::Draw(GL_Shader& shader) {
        glBindVertexArray(vao);

        if (textures.size()) {
            glBindTextureUnit(0, textures[0].id);
            unsigned int loc = shader.GetUniformLoc("uTex2D");
            glUniform1i(loc, 0);
        }

        glm::mat4 s = glm::scale(glm::mat4(1.f), { 1.f, 1.f, 1.f });
        //s = s * transform;
        GLuint uniform = shader.GetUniformLoc("M");
        glUniformMatrix4fv(uniform, 1, false, glm::value_ptr(s));

        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
    }


    void Vertex::SetBonesDefault() {
        for (int i{}; i < MAX_BONE_INFLUENCE; ++i) {
            boneIDs[i] = -1;
            weights[i] = 0.f;
        }
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

        fs.write(buffer, buffer_size);
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

        source = sizeof(Texture);
        memcpy(buffer + offset, &source, i_size); offset += i_size;
        source = textures.size();
        memcpy(buffer + offset, &source, i_size); offset += i_size;

        //buffer
        uint64_t vert_buffer_size = vertices.size() * sizeof(Vertex);
        uint64_t idx_buffer_size = indices.size() * sizeof(unsigned int);
        uint64_t tex_buffer_size = textures.size() * sizeof(Texture);

        //premultiply all the positions and normals because we are collapsing the hierachy in this example
        std::vector<Vertex> new_vertices = vertices;
        for (auto& v : new_vertices) {  //convert ctor from vec4 to vec3
            v.position = transform * glm::vec4(v.position, 1.0f);
            v.normal = transform * glm::vec4(v.normal, 1.0f);
        }
        //note that in this version, using new_vertices.data instead of vertices.data
        memcpy(buffer + offset, new_vertices.data(), vert_buffer_size); offset += vert_buffer_size;
        memcpy(buffer + offset, indices.data(), idx_buffer_size); offset += idx_buffer_size;
        memcpy(buffer + offset, textures.data(), tex_buffer_size); offset += tex_buffer_size;
        memcpy(buffer + offset, glm::value_ptr(transform), sizeof(glm::mat4x4)); offset += sizeof(glm::mat4x4);
    }

    void Mesh::unpack_data(char* const buffer, uint64_t& offset) {
        uint32_t dest{};

        //name
        memcpy(&dest, buffer + offset, i_size); offset += i_size;
        name.resize(dest);
        memcpy(name.data(), buffer + offset, dest); offset += dest;

        uint32_t vert_size{}, idx_size{}, tex_size{};
        uint32_t num_vert{}, num_idx{}, num_tex{};

        //buffer details
        memcpy(&vert_size, buffer + offset, i_size); offset += i_size;
        memcpy(&num_vert, buffer + offset, i_size); offset += i_size;

        memcpy(&idx_size, buffer + offset, i_size); offset += i_size;
        memcpy(&num_idx, buffer + offset, i_size); offset += i_size;

        memcpy(&tex_size, buffer + offset, i_size); offset += i_size;
        memcpy(&num_tex, buffer + offset, i_size); offset += i_size;

        //buffer
        uint64_t vert_buffer_size = vert_size * num_vert;
        uint64_t idx_buffer_size = idx_size * num_idx;
        uint64_t tex_buffer_size = tex_size * num_tex;

        vertices.resize(num_vert);
        indices.resize(num_idx);
        textures.resize(num_tex);
        memcpy(vertices.data(), buffer + offset, vert_buffer_size); offset += vert_buffer_size;
        memcpy(indices.data(), buffer + offset, idx_buffer_size); offset += idx_buffer_size;
        memcpy(textures.data(), buffer + offset, tex_buffer_size); offset += tex_buffer_size;
        memcpy(glm::value_ptr(transform), buffer + offset, sizeof(glm::mat4x4)); offset += sizeof(glm::mat4x4);
    }

    uint64_t Model::get_model_size() const {
        uint64_t size{};

        size = i_size + name.size() + i_size;    //name size + name + mesh count

        for (auto& m : meshes) {
            size += m.get_mesh_size();
        }

        return size;
    }
    uint64_t Mesh::get_mesh_size() const {
        uint64_t size{};

        size = i_size +                         //name size
            name.size() +                       //name]
            i_size +                            //vtx size
            i_size +                            //num vtx
            i_size +                            //idx size
            i_size +                            //num idx
            i_size +                            //tex size
            i_size +                            //num tex
            vertices.size() * sizeof(Vertex) +  //vtx
            indices.size() * i_size +           //idx
            textures.size() * sizeof(Texture) + //tex
            sizeof(glm::mat4x4);                //tform

        return size;
    }
}