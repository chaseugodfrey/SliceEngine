

#include "Assimp_Importer.h"
#if !COMPILE_ONLY
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <gli.hpp>
#include "Camera.h"
#include "OpenGL_Starter.h"

#include "Animation.h"

#include <filesystem>

namespace {
    GLFWwindow* window{};

    unsigned int width = 600;
    unsigned int height = 600;

    void GetShaderError(GLuint shaderHandle);
    void GetProgramError(GLuint programHandle);

    GLuint LoadTextureFile(std::string const&);
    GLuint curr_texture{};

    Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
    float lastX = width / 2.0f;
    float lastY = height / 2.0f;
    bool firstMouse = true;

    void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    void processInput(GLFWwindow* window);
}

void OpenGL_Starter::Init() {
    //Windows and Wrangler extensions
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    window = glfwCreateWindow(width, height, "OpenGL Proj", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Glew Error: " << glewGetErrorString(err) << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    //Opengl
    glViewport(0, 0, width, height);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    const char* model_path = "../Asset/player.fbx";
    //const char* model_path = "../Asset/dancing_vampire.dae";
    const char* mdl_asset_path = "../Asset/player.mdl";
    const char* anim_path = "../Asset/player_anim.animpkg";
    const char* skel_path = "../Asset/player_skel.skl";

    assert(std::filesystem::exists(model_path));
    //mdl.load_static_model(model_path);
    skeleton.load(model_path);
    skeleton.Save_Skeleton(skel_path);
    skeleton_clone.Load_Skeleton(skel_path);

    mdl.load_skinned_model(model_path, skeleton_clone);
    mdl.SaveModelToFile(mdl_asset_path);
    mdl_clone.LoadModelFromFile(mdl_asset_path);
    mdl_clone.InitLoadedModel();

    anim_package.Load(model_path, skeleton_clone);
    anim_package.SavePackage(anim_path);

    anim_clone.LoadPackage(anim_path);

    animator.animations = &anim_clone;
    animator.skeleton = &skeleton_clone;
    animator.Play(0);
    animator.root_node = &mdl_clone.root_node;
    //anim.Init(model_path);
    //animator.PlayAnimation(&anim);
    //mdl.SaveModelToFile("../Asset/player_data.mdl");

  /*  if (mdl_clone.LoadModelFromFile("../Asset/player_data.mdl")) {
        mdl_clone.InitLoadedModel();
        mdl_clone.SaveModelToFile("../Asset/player_data_clone.mdl");
    }*/
  /*  if (mdl_clone.LoadModelFromFile("../Asset/cube.mdl")) {
        mdl_clone.InitLoadedModel();
    }*/

    //Shaders
    //const char* vert_name = "DefaultShader.vert";
    const char* vert_name = "SkinShader.vert";
    const char* frag_name = "DefaultShader.frag";
    //Read shader files
    default_shader.Compile_Link_Validate(vert_name, frag_name);
    CheckGLError();

    //Texture Related

    //Load the compressed texture
    curr_texture = LoadTextureFile("../Asset/5271507727521808385.dds");
    glBindTextureUnit(1, curr_texture);
    CheckGLError();


    //Rendering related
    glClearColor(0, 0, 0, 0);
}

void OpenGL_Starter::Update() {
    animator.Update(0);
    while (!glfwWindowShouldClose(window)) {
        static float time{-1.f};
        
        float dt{};
        if (time < 0.f) {
            time = glfwGetTime();
        }
        else {
            float curr = glfwGetTime();
            dt = curr - time;
            time = curr;
        }

        animator.Update(dt);

        processInput(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(default_shader.handle);
        //glBindVertexArray(default_quad.vao_id);
        GLint uniform = glGetUniformLocation(default_shader.handle, "uTex2D");
        glUniform1i(uniform, 1);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        uniform = default_shader.GetUniformLoc("P");
        glUniformMatrix4fv(uniform, 1, false, glm::value_ptr(projection));
        uniform = default_shader.GetUniformLoc("V");
        glUniformMatrix4fv(uniform, 1, false, glm::value_ptr(view));

        uniform = default_shader.GetUniformLoc("light_pos");
        glUniform3fv(uniform, 1, glm::value_ptr(camera.Position));

        uniform = default_shader.GetUniformLoc("finalBonesMatrices");
        glUniformMatrix4fv(uniform, MAX_BONES, false, glm::value_ptr(animator.final_transforms.data()[0]));

        // render the loaded model
        //std::cout << animator.current_time << std::endl;

        /*
        * animator.play
        * animator.updatetransform(mdl)
        * mdl.draw
        */
        //mdl.Draw(default_shader);

        mdl_clone.Draw(default_shader);

        //float initialOffsetY = -0.5f;
        //GLuint uni_loc = default_shader.GetUniformLoc("offset");
        //for (int i = 1; i < 2; ++i) {
        //    //Create the offset vector
        //    glm::vec2 offset{};
        //    float scaleFactor = 1.f / ((float)i * 2);

        //    offset.x = scaleFactor;
        //    offset.y = initialOffsetY;
        //    initialOffsetY += scaleFactor * 0.5f + 1.f / ((float)(i + 1) * 4);

        //    glUniform2f(uni_loc, offset.x, offset.y);

        //    glDrawArrays(default_quad.primitive_type, 0, default_quad.draw_cnt);
        //}



        glfwSwapBuffers(window);
        glfwPollEvents();

        CheckGLError();
    }
}

void OpenGL_Starter::Terminate() {
    default_shader.Destroy();
    mdl.Release();
    //glDeleteBuffers(1, &default_quad.vbo_id);
    //glDeleteVertexArrays(1, &default_quad.vao_id);
    glDeleteTextures(1, &curr_texture);

    CheckGLError();
    glfwTerminate();
}

namespace {
    void GetShaderError(GLuint shader_handle) {
        GLint log_len;
        std::string log_string{};
        glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &log_len);
        if (log_len > 0) {
            GLchar* log = new GLchar[log_len];
            GLsizei written_log_len;
            glGetShaderInfoLog(shader_handle, log_len, &written_log_len, log);
            log_string += log;
            delete[] log;
        }
        std::cout << log_string << std::endl;
    }
    void GetProgramError(GLuint program_handle) {
        GLint log_len;
        std::string log_string{};
        glGetProgramiv(program_handle, GL_INFO_LOG_LENGTH, &log_len);
        if (log_len > 0) {
            GLchar* log = new GLchar[log_len];
            GLsizei written_log_len;
            glGetProgramInfoLog(program_handle, log_len, &written_log_len, log);
            log_string += log;
            delete[] log;
        }
        std::cout << log_string << std::endl;
    }


    GLuint LoadTextureFile(std::string const& filepath) {
        gli::texture Texture = gli::load(filepath);
        if (Texture.empty()) {
            return 0;
        }

        gli::gl GL(gli::gl::PROFILE_GL33);
        gli::gl::format const format = GL.translate(Texture.format(), Texture.swizzles());
        GLenum target = GL.translate(Texture.target());

        GLuint texture_id{};
        glCreateTextures(target, 1, &texture_id);
        glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, static_cast<GLuint>(Texture.levels() - 1));
        glTexParameteri(target, GL_TEXTURE_SWIZZLE_R, format.Swizzles[0]);
        glTexParameteri(target, GL_TEXTURE_SWIZZLE_G, format.Swizzles[1]);
        glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, format.Swizzles[2]);
        glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, format.Swizzles[3]);
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        CheckGLError();
        //get extent at mipmap level 0(base)
        glm::tvec3<GLsizei> const extent(Texture.extent(0));
        //2d vs 1d array/cube -> facetotal vs extent
        //3d vs 2d array/cube array -> facetotal vs extent
        GLsizei const faceTotal = static_cast<GLsizei>(Texture.layers() * Texture.faces());
        CheckGLError();
        switch (Texture.target()) {
        case gli::TARGET_1D:
            glTextureStorage1D(texture_id, static_cast<GLint>(Texture.levels()), format.Internal, extent.x);
            break;
        case gli::TARGET_1D_ARRAY:
        case gli::TARGET_2D:
        case gli::TARGET_CUBE:
            glTextureStorage2D(texture_id, static_cast<GLint>(Texture.levels()), format.Internal,
                extent.x, Texture.target() == gli::TARGET_2D ? extent.y : faceTotal);
            break;
        case gli::TARGET_2D_ARRAY:
        case gli::TARGET_3D:
        case gli::TARGET_CUBE_ARRAY:
            glTextureStorage3D(texture_id, static_cast<GLint>(Texture.levels()), format.Internal,
                extent.x, extent.y, Texture.target() == gli::TARGET_3D ? extent.z : faceTotal);
        }
        CheckGLError();

        for (size_t layer = 0; layer < Texture.layers(); ++layer) {
            for (size_t face = 0; face < Texture.faces(); ++face) {
                for (size_t level = 0; level < Texture.levels(); ++level) {
                    glm::tvec3<GLsizei> level_extent = Texture.extent(level);
                    GLenum curr_Target = gli::is_target_cube(Texture.target())
                        ? static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face)
                        : target;
                    switch (Texture.target()) {
                    case gli::TARGET_1D:
                        if (gli::is_compressed(Texture.format())) {
                            glCompressedTextureSubImage1D(texture_id, level, 0, level_extent.x, format.Internal,
                                static_cast<GLsizei>(Texture.size(level)), Texture.data(layer, face, level));
                        }
                        else {
                            glTextureSubImage1D(texture_id, level, 0, level_extent.x, format.External,
                                format.Type, Texture.data(layer, face, level));
                        }
                        CheckGLError();
                        break;
                    case gli::TARGET_2D:
                    case gli::TARGET_1D_ARRAY:
                    case gli::TARGET_CUBE:
                        if (gli::is_compressed(Texture.format())) {
                            glCompressedTextureSubImage2D(texture_id, level, 0, 0, level_extent.x,
                                target == gli::TARGET_1D_ARRAY ? layer : level_extent.y, format.Internal,
                                static_cast<GLsizei>(Texture.size(level)), Texture.data(layer, face, level));
                        }
                        else {
                            glTextureSubImage2D(texture_id, level, 0, 0, level_extent.x,
                                target == gli::TARGET_1D_ARRAY ? layer : level_extent.y,
                                format.External, format.Type, Texture.data(layer, face, level));
                        }
                        CheckGLError();
                        break;
                    case gli::TARGET_2D_ARRAY:
                    case gli::TARGET_3D:
                    case gli::TARGET_CUBE_ARRAY:
                        if (gli::is_compressed(Texture.format())) {
                            glCompressedTextureSubImage3D(texture_id, level, 0, 0, 0,
                                level_extent.x, level_extent.y, Texture.target() == gli::TARGET_3D ? level_extent.z : layer,
                                format.Internal, static_cast<GLsizei>(Texture.size(level)),
                                Texture.data(layer, face, level));
                        }
                        else {
                            glTextureSubImage3D(texture_id, level, 0, 0, 0,
                                level_extent.x, level_extent.y, Texture.target() == gli::TARGET_3D ? level_extent.z : layer,
                                format.External, format.Type,
                                Texture.data(layer, face, level));
                        }
                        CheckGLError();
                        break;
                    default:
                        assert(0);
                        break;
                    }
                }
            }
        }

        CheckGLError();
        return texture_id;
    }

    // process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
 // ---------------------------------------------------------------------------------------------------------
    void processInput(GLFWwindow* window)
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, 1.f/60.f);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, 1.f / 60.f);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, 1.f / 60.f);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, 1.f / 60.f);
    }

    // glfw: whenever the window size changed (by OS or user resize) this callback function executes
    // ---------------------------------------------------------------------------------------------
    void framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
        // make sure the viewport matches the new window dimensions; note that width and 
        // height will be significantly larger than specified on retina displays.
        glViewport(0, 0, width, height);
    }

    // glfw: whenever the mouse moves, this callback is called
    // -------------------------------------------------------
    void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
    {
        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);

        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

        lastX = xpos;
        lastY = ypos;

        camera.ProcessMouseMovement(xoffset, yoffset);
    }

    // glfw: whenever the mouse scroll wheel scrolls, this callback is called
    // ----------------------------------------------------------------------
    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        camera.ProcessMouseScroll(static_cast<float>(yoffset));
    }
} 


void _CheckGLError(const char* file, int line)
{
#ifndef _DEBUG
    return;
#endif // only do this on debug

    GLenum err(glGetError());

    while (err != GL_NO_ERROR)
    {
        std::string error;
        switch (err)
        {
        case GL_INVALID_OPERATION:  error = "INVALID_OPERATION";      break;
        case GL_INVALID_ENUM:       error = "INVALID_ENUM";           break;
        case GL_INVALID_VALUE:      error = "INVALID_VALUE";          break;
        case GL_OUT_OF_MEMORY:      error = "OUT_OF_MEMORY";          break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
        }
        std::cout << "GL_" << error.c_str() << " - " << file << ":" << line << std::endl;
        err = glGetError();
    }

    return;
}
#endif