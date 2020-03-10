#pragma once

#include <string>
#include <utility>

#include <glad/glad.h>

#include <cgl/error.hpp>

#include <util.hpp>

namespace cgl {
    template<GLenum mode>
    class shader {
    protected:
        GLuint id{};

    public:
        shader() {
            id = glCreateShader(mode);
        }

        shader(const std::string &src) : shader() {
            set_source(src);

            if (!compile())
                throw shader_error(get_info_log());
        }

        static shader<mode> file(const std::string &name) {
            return shader<mode>(utilReadFile(name));
        }

        shader(shader &) = delete;

        shader(shader &&o) noexcept {
            id = std::exchange(o.id, 0);
        };

        ~shader() {
            glDeleteShader(id);
        }

        operator GLuint() const {
            return id;
        }

        [[nodiscard]] int get(GLenum pname) const {
            GLint res;
            glGetShaderiv(id, pname, &res);
            return (int) res;
        }

        [[nodiscard]] std::string get_info_log() const {
            auto len = (size_t) get(GL_INFO_LOG_LENGTH);
            char buffer[len];
            glGetShaderInfoLog(id, len, nullptr, buffer);
            return std::string(buffer);
        }

        void set_source(const std::string &src) {
            const char *c_src = src.c_str();
            glShaderSource(id, 1, &c_src, nullptr);
        }

        bool compile() {
            glCompileShader(id);
            return (bool) get(GL_COMPILE_STATUS);
        }
    };

    namespace sh {
        using vert = shader<GL_VERTEX_SHADER>;
        using tcs = shader<GL_TESS_CONTROL_SHADER>;
        using tes = shader<GL_TESS_EVALUATION_SHADER>;
        using geom = shader<GL_GEOMETRY_SHADER>;
        using frag = shader<GL_FRAGMENT_SHADER>;
        using comp = shader<GL_COMPUTE_SHADER>;
    }
}
