#pragma once

#include <glad/glad.h>

#include <string>
#include <memory>
#include <vector>

template<GLenum mode>
class Shader {
public:
private:
    GLuint id = 0;

public:
    explicit Shader(const std::string &src) {
        id = glCreateShader(mode);

        const char *str = src.c_str();
        glShaderSource(id, 1, &str, nullptr);
        glCompileShader(id);

        // todo throw if compile failed
    }

    explicit Shader(std::ifstream source)
        : Shader(std::string(
        std::istreambuf_iterator<char>(source),
        std::istreambuf_iterator<char>()
    )) {}

    Shader(const Shader &) = delete;

    Shader(Shader &&o) noexcept {
        id = std::exchange(o.id, 0);
    }

    ~Shader() {
        glDeleteShader(id);
    }

    operator GLuint() const { // NOLINT(google-explicit-constructor)
        return id;
    }
};

using VertexShader = Shader<GL_VERTEX_SHADER>;
using FragmentShader = Shader<GL_FRAGMENT_SHADER>;

class Program {
private:
    GLuint id = 0;

public:
    template<GLenum ...mode>
    explicit Program(const Shader<mode> &...shader) {
        id = glCreateProgram();

        (glAttachShader(id, shader), ...);

        glLinkProgram(id);

        // todo throw if link failed
    }

    Program(const Program &) = delete;

    Program(Program &&o) noexcept {
        id = std::exchange(o.id, 0);
    }

    ~Program() {
        glDeleteProgram(id);
    }

    operator GLuint() const { // NOLINT(google-explicit-constructor)
        return id;
    }
};
