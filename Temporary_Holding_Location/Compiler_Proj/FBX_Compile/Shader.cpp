
#include "Shader.h"
#include "Assimp_Importer.h"
#if !COMPILE_ONLY

#include <iostream>
#include <string>
#include <gl/glew.h>
#include <fstream>
#include <sstream>
#include <cassert>
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

}


void GL_Shader::Compile_Link_Validate(const char* vert_name, const char* frag_name) {
    std::ifstream vert_stream(vert_name, std::ifstream::in);
    if (!vert_stream) {
        std::cout << "Unable to open file: " << vert_name << std::endl;
        return;
    }
    std::ifstream frag_stream(frag_name, std::ifstream::in);
    if (!frag_stream) {
        std::cout << "Unable to open file: " << frag_name << std::endl;
        return;
    }

    std::stringstream vert_ss;
    vert_ss << vert_stream.rdbuf();
    std::stringstream frag_ss;
    frag_ss << frag_stream.rdbuf();

    vert_stream.close();
    frag_stream.close();

    //Create shader objects
    GLint status;
    handle = glCreateProgram();
    unsigned int vert_obj = glCreateShader(GL_VERTEX_SHADER);
    std::string t_string = vert_ss.str();
    GLchar const* vert_code[] = { t_string.c_str() };
    glShaderSource(vert_obj, 1, vert_code, NULL);
    glCompileShader(vert_obj);

    glGetShaderiv(vert_obj, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GetShaderError(vert_obj);
    }
    assert(status != GL_FALSE && "Failed to compile vertex shader");
    glAttachShader(handle, vert_obj);

    unsigned int frag_obj = glCreateShader(GL_FRAGMENT_SHADER);
    t_string = frag_ss.str();
    GLchar const* frag_code[] = { t_string.c_str() };
    glShaderSource(frag_obj, 1, frag_code, NULL);
    glCompileShader(frag_obj);

    glGetShaderiv(frag_obj, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GetShaderError(frag_obj);
    }
    assert(status != GL_FALSE && "Failed to compile fragment shader");
    glAttachShader(handle, frag_obj);

    glLinkProgram(handle);
    glGetProgramiv(handle, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        GetProgramError(handle);
    }
    assert(status != GL_FALSE && "Failed to link shader program");

    glValidateProgram(handle);
    glGetProgramiv(handle, GL_VALIDATE_STATUS, &status);
    if (status == GL_FALSE) {
        GetProgramError(handle);
    }
    assert(status != GL_FALSE && "Failed to validate shader program");

    glDetachShader(handle, frag_obj);
    glDetachShader(handle, vert_obj);
    glDeleteShader(frag_obj);
    glDeleteShader(vert_obj);
}

void GL_Shader::Destroy() {
    glDeleteProgram(handle);
}

unsigned int GL_Shader::GetUniformLoc(const char* uniformName) const {
    return glGetUniformLocation(handle, uniformName);
}
#endif