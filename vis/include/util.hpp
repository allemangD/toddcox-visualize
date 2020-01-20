#pragma once

#include <cerrno>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <glad/glad.h>

class gl_error : public std::domain_error {
public:
    explicit gl_error(const std::string &arg) : domain_error(arg) {}

    explicit gl_error(const char *string) : domain_error(string) {}
};

class shader_error : public gl_error {
public:
    explicit shader_error(const std::string &arg) : gl_error(arg) {}

    explicit shader_error(const char *string) : gl_error(string) {}
};

class program_error : public gl_error {
public:
    explicit program_error(const std::string &arg) : gl_error(arg) {}

    explicit program_error(const char *string) : gl_error(string) {}
};

void utilShaderSource(GLuint shader, const std::vector<std::string> &sources) {
    char const *ptrs[sources.size()];
    for (size_t i = 0; i < sources.size(); ++i) {
        ptrs[i] = sources[i].c_str();
    }
    glShaderSource(shader, sources.size(), ptrs, nullptr);
}

std::string utilShaderInfoLog(GLuint shader) {
    int len;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    char buffer[len];
    glGetShaderInfoLog(shader, len, nullptr, buffer);
    return std::string(buffer);
}

std::string utilProgramInfoLog(GLuint program) {
    int len;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
    char buffer[len];
    glGetProgramInfoLog(program, len, nullptr, buffer);
    return std::string(buffer);
}

std::string utilInfo() {
    std::stringstream ss;
    ss
        << "Graphics Information:" << std::endl
        << "    Vendor:          " << glGetString(GL_VENDOR) << std::endl
        << "    Renderer:        " << glGetString(GL_RENDERER) << std::endl
        << "    OpenGL version:  " << glGetString(GL_VERSION) << std::endl
        << "    Shading version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    return ss.str();
}

std::string utilReadFile(const std::string &filename) {
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (in) {
        std::ostringstream contents;
        contents << in.rdbuf();
        in.close();
        return (contents.str());
    }
    throw std::system_error(errno, std::generic_category());
}

GLuint utilCompileFiles(const GLenum type, const std::vector<std::string> &files) {
    std::vector<std::string> sources;
    sources.reserve(files.size());
    for (const auto &file : files) {
        sources.push_back(utilReadFile(file));
    }

    GLuint shader = glCreateShader(type);
    utilShaderSource(shader, sources);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success) return shader;

    throw shader_error(utilShaderInfoLog(shader));
}

GLuint utilLinkProgram(const std::vector<GLuint> &shaders) {
    GLuint program = glCreateProgram();
    for (const auto &shader : shaders) {
        glAttachShader(program, shader);
    }
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success)return program;

    throw program_error(utilProgramInfoLog(program));
}
