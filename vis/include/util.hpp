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

template<class T, GLenum prop>
T utilGetShader(GLuint shader) {
    GLint res;
    glGetShaderiv(shader, prop, &res);
    return static_cast<T>(res);
}

#define getShaderInfoLogLength utilGetShader<size_t, GL_INFO_LOG_LENGTH>
#define getShaderCompileStatus utilGetShader<size_t, GL_COMPILE_STATUS>

template<class T, GLenum prop>
T utilGetProgram(GLuint program) {
    GLint res;
    glGetProgramiv(program, prop, &res);
    return static_cast<T>(res);
}

#define getProgramInfoLogLength utilGetProgram<size_t, GL_INFO_LOG_LENGTH>
#define getProgramLinkStatus utilGetProgram<size_t, GL_LINK_STATUS>

void utilShaderSource(GLuint shader, const std::vector<std::string> &sources) {
    char const *ptrs[sources.size()];
    for (size_t i = 0; i < sources.size(); ++i) {
        ptrs[i] = sources[i].c_str();
    }
    glShaderSource(shader, sources.size(), ptrs, nullptr);
}

std::string getShaderInfoLog(GLuint shader) {
    int len = getShaderInfoLogLength(shader);
    char buffer[len];
    glGetShaderInfoLog(shader, len, nullptr, buffer);
    return std::string(buffer);
}

std::string getProgramInfoLog(GLuint program) {
    int len = getProgramInfoLogLength(program);
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

    if (getShaderCompileStatus(shader)) return shader;

    throw shader_error(getShaderInfoLog(shader));
}

GLuint utilLinkProgram(const std::vector<GLuint> &shaders) {
    GLuint program = glCreateProgram();
    for (const auto &shader : shaders) {
        glAttachShader(program, shader);
    }
    glLinkProgram(program);

    if (getProgramLinkStatus(program)) return program;

    throw program_error(getProgramInfoLog(program));
}

GLuint utilCreateShaderProgram(GLenum type, const std::vector<std::string> &src) {
    std::vector<const char *> c_str(src.size());
    std::transform(src.begin(), src.end(), c_str.begin(), [](auto &str) {
        return str.c_str();
    });

    GLuint program = glCreateShaderProgramv(type, src.size(), &c_str[0]);

    if (getProgramLinkStatus(program)) return program;

    throw program_error(getProgramInfoLog(program));
}

GLuint utilCreateShaderProgramFile(GLenum type, const std::vector<std::string> &files) {
    std::vector<std::string> sources(files.size());
    std::transform(files.begin(), files.end(), sources.begin(), utilReadFile);
    return utilCreateShaderProgram(type, sources);
}

std::vector<GLuint> utilCreateVertexArrays(int n) {
    std::vector<GLuint> res(n);
    glCreateVertexArrays(n, &res[0]);
    return res;
}

GLuint utilCreateVertexArray() {
    return utilCreateVertexArrays(1)[0];
}

std::vector<GLuint> utilCreateBuffers(int n) {
    std::vector<GLuint> res(n);
    glCreateBuffers(n, &res[0]);
    return res;
}

GLuint utilCreateBuffer() {
    return utilCreateBuffers(1)[0];
}