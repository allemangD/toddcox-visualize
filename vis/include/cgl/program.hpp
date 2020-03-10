#pragma once

#include <string>
#include <utility>

#include <glad/glad.h>

#include <cgl/error.hpp>
#include <cgl/shader.hpp>

#include <util.hpp>

namespace cgl {
    class Program {
    protected:
        GLuint id{};

    public:
        Program() {
            id = glCreateProgram();
        }

        Program(Program &) = delete;

        Program(Program &&o) noexcept {
            id = std::exchange(o.id, 0);
        };

        ~Program() {
            glDeleteProgram(id);
        }

        operator GLuint() const {
            return id;
        }

        [[nodiscard]] int get(GLenum pname) const {
            GLint res;
            glGetProgramiv(id, pname, &res);
            return (int) res;
        }

        [[nodiscard]] std::string get_info_log() const {
            auto len = (size_t) get(GL_INFO_LOG_LENGTH);
            char buffer[len];
            glGetProgramInfoLog(id, len, nullptr, buffer);
            return std::string(buffer);
        }

        template<GLenum mode>
        void attach(const Shader<mode> &sh) {
            glAttachShader(id, sh);
        }

        template<GLenum mode>
        void detach(const Shader<mode> &sh) {
            glDetachShader(id, sh);
        }

        bool link() {
            glLinkProgram(id);
            return (bool) get(GL_LINK_STATUS);
        }
    };
}