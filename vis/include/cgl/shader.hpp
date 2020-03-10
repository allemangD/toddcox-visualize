#pragma once

#include <string>
#include <utility>

#include <glad/glad.h>

#include <cgl/error.hpp>

#include <util.hpp>

namespace cgl {
    template<GLenum mode>
    class Shader {
    protected:
        GLuint id{};

    public:
        Shader() {
            id = glCreateShader(mode);
        }

        Shader(const std::string &src) : Shader() {
            set_source(src);

            if (!compile())
                throw ShaderError(get_info_log());
        }

        static Shader<mode> file(const std::string &name) {
            return Shader<mode>(utilReadFile(name));
        }

        Shader(Shader &) = delete;

        Shader(Shader &&o) noexcept {
            id = std::exchange(o.id, 0);
        };

        ~Shader() {
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
        using vert = Shader<GL_VERTEX_SHADER>;
        using tcs = Shader<GL_TESS_CONTROL_SHADER>;
        using tes = Shader<GL_TESS_EVALUATION_SHADER>;
        using geom = Shader<GL_GEOMETRY_SHADER>;
        using frag = Shader<GL_FRAGMENT_SHADER>;
        using comp = Shader<GL_COMPUTE_SHADER>;
    }
}
