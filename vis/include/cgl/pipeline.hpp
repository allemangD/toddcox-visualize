#pragma once

#include <functional>
#include <string>
#include <utility>

#include <glad/glad.h>

#include <cgl/error.hpp>
#include <cgl/shaderprogram.hpp>

#include <util.hpp>

namespace cgl {
    class pipeline {
    protected:
        GLuint id{};

    public:
        pipeline() {
            glCreateProgramPipelines(1, &id);
        }

        pipeline(pipeline &) = delete;

        pipeline(pipeline &&o) noexcept {
            id = std::exchange(o.id, 0);
        }

        ~pipeline() {
            glDeleteProgramPipelines(1, &id);
            id = 0;
        }

        operator GLuint() const {
            return id;
        }

        [[nodiscard]] int get(GLenum pname) const {
            GLint res;
            glGetProgramPipelineiv(id, pname, &res);
            return (int) res;
        }

        [[nodiscard]] std::string get_info_log() const {
            auto len = (size_t) get(GL_INFO_LOG_LENGTH);
            char buffer[len];
            glGetProgramPipelineInfoLog(id, len, nullptr, buffer);
            return std::string(buffer);
        }

        pipeline &unstage(GLenum stage_bits) {
            glUseProgramStages(id, stage_bits, 0);
            return *this;
        }

        pipeline &stage(const ShaderProgram<GL_VERTEX_SHADER> &pgm) {
            glUseProgramStages(id, GL_VERTEX_SHADER_BIT, pgm);
            return *this;
        }

        pipeline &stage(const ShaderProgram<GL_TESS_CONTROL_SHADER> &pgm) {
            glUseProgramStages(id, GL_TESS_CONTROL_SHADER_BIT, pgm);
            return *this;
        }

        pipeline &stage(const ShaderProgram<GL_TESS_EVALUATION_SHADER> &pgm) {
            glUseProgramStages(id, GL_TESS_EVALUATION_SHADER_BIT, pgm);
            return *this;
        }

        pipeline &stage(const ShaderProgram<GL_GEOMETRY_SHADER> &pgm) {
            glUseProgramStages(id, GL_GEOMETRY_SHADER_BIT, pgm);
            return *this;
        }

        pipeline &stage(const ShaderProgram<GL_FRAGMENT_SHADER> &pgm) {
            glUseProgramStages(id, GL_FRAGMENT_SHADER_BIT, pgm);
            return *this;
        }

        pipeline &stage(const ShaderProgram<GL_COMPUTE_SHADER> &pgm) {
            glUseProgramStages(id, GL_COMPUTE_SHADER_BIT, pgm);
            return *this;
        }

        void bound(const std::function<void()> &action) const {
            glBindProgramPipeline(id);
            action();
            glBindProgramPipeline(0);
        }
    };
}