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
    class shaderprogram : public program {
    public:
        shaderprogram() : program() {
            glProgramParameteri(id, GL_PROGRAM_SEPARABLE, GL_TRUE);
        }

        shaderprogram(const std::string &src) : shaderprogram() {
            shader<mode> sh(src);

            attach(sh);

            if (!link())
                throw shader_error(get_info_log());

            detach(sh);
        }

        static shaderprogram<mode> file(const std::string &name) {
            return shaderprogram<mode>(utilReadFile(name));
        }
    };

    namespace pgm {
        using vert = shaderprogram<GL_VERTEX_SHADER>;
        using tcs = shaderprogram<GL_TESS_CONTROL_SHADER>;
        using tes = shaderprogram<GL_TESS_EVALUATION_SHADER>;
        using geom = shaderprogram<GL_GEOMETRY_SHADER>;
        using frag = shaderprogram<GL_FRAGMENT_SHADER>;
        using comp = shaderprogram<GL_COMPUTE_SHADER>;
    }
}