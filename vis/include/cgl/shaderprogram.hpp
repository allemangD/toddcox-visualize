#pragma once

#include <string>
#include <utility>

#include <glad/glad.h>

#include <cgl/error.hpp>
#include <cgl/shader.hpp>
#include <cgl/program.hpp>

#include <util.hpp>

namespace cgl{
    template<GLenum mode>
    class ShaderProgram : public Program {
    public:
        ShaderProgram() : Program() {
            glProgramParameteri(id, GL_PROGRAM_SEPARABLE, GL_TRUE);
        }

        ShaderProgram(const std::string &src) : ShaderProgram() {
            Shader<mode> sh(src);

            attach(sh);

            if (!link())
                throw ShaderError(get_info_log());

            detach(sh);
        }

        static ShaderProgram<mode> file(const std::string &name) {
            return ShaderProgram<mode>(utilReadFile(name));
        }
    };

    namespace pgm {
        using vert = ShaderProgram<GL_VERTEX_SHADER>;
        using tcs = ShaderProgram<GL_TESS_CONTROL_SHADER>;
        using tes = ShaderProgram<GL_TESS_EVALUATION_SHADER>;
        using geom = ShaderProgram<GL_GEOMETRY_SHADER>;
        using frag = ShaderProgram<GL_FRAGMENT_SHADER>;
        using comp = ShaderProgram<GL_COMPUTE_SHADER>;
    }
}