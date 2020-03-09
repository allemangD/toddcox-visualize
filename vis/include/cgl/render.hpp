#pragma once

#include <memory>

#include <glad/glad.h>

namespace cgl {
    template<class T>
    class buffer {
        GLuint id{};

    public:
        buffer() {
            glCreateBuffers(1, &id);
        }

        buffer(const T &data, GLenum usage = GL_STATIC_DRAW)
            : buffer() {
            put(data, usage);
        }

        buffer(const std::vector<T> &data, GLenum usage = GL_STATIC_DRAW)
            : buffer() {
            put(data, usage);
        }

        buffer(buffer &) = delete;

        buffer(buffer &&o) noexcept {
            id = std::exchange(o.id, 0);
        };

        ~buffer() {
            glDeleteBuffers(1, &id);
            id = 0;
        }

        operator GLuint() const {
            return id;
        }

        void put(const T &data, GLenum usage = GL_STATIC_DRAW) {
            glNamedBufferData(id, sizeof(T), &data, usage);
        }

        void put(const std::vector<T> &data, GLenum usage = GL_STATIC_DRAW) {
            glNamedBufferData(id, sizeof(T) * data.size(), &data[0], usage);
        }
    };

    class shader {
    protected:
        GLuint id{};
        GLenum mode;

    public:
        shader(GLenum mode) : mode(mode) {
            id = glCreateShader(mode);
        }

        shader(shader &) = delete;

        shader(shader &&o) noexcept {
            mode = o.mode;
            id = std::exchange(o.id, 0);
        };

        ~shader() {
            glDeleteShader(id);
        }

        operator GLuint() const {
            return id;
        }

        [[nodiscard]] GLenum get_mode() const {
            return mode;
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

    class program {
    protected:
        GLuint id{};

    public:
        program() {
            id = glCreateProgram();
        }

        program(program &) = delete;

        program(program &&o) noexcept {
            id = std::exchange(o.id, 0);
        };

        ~program() {
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

        void attach(const shader &sh) {
            glAttachShader(id, sh);
        }

        void detach(const shader &sh) {
            glDetachShader(id, sh);
        }

        bool link() {
            glLinkProgram(id);
            return (bool) get(GL_LINK_STATUS);
        }
    };

    class shaderprogram : public program {
    protected:
        GLenum mode;

    public:
        shaderprogram(GLenum mode) : program(), mode(mode) {
            glProgramParameteri(id, GL_PROGRAM_SEPARABLE, GL_TRUE);
        }

        shaderprogram(shaderprogram &&o) noexcept : program(std::move(o)) {
            mode = o.mode;
        }

        [[nodiscard]] GLenum get_mode() const {
            return mode;
        }
    };

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

        void use_stages(const shaderprogram &pgm) {
            GLbitfield bits;
            switch (pgm.get_mode()) {
            case GL_VERTEX_SHADER:
                bits = GL_VERTEX_SHADER_BIT;
                break;
            case GL_TESS_CONTROL_SHADER:
                bits = GL_TESS_CONTROL_SHADER_BIT;
                break;
            case GL_TESS_EVALUATION_SHADER:
                bits = GL_TESS_EVALUATION_SHADER_BIT;
                break;
            case GL_GEOMETRY_SHADER:
                bits = GL_GEOMETRY_SHADER_BIT;
                break;
            case GL_FRAGMENT_SHADER:
                bits = GL_FRAGMENT_SHADER_BIT;
                break;
            }

            glUseProgramStages(id, bits, pgm);
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
    };

    shader compile_shader(GLenum mode, const std::string &src) {
        shader res(mode);

        res.set_source(src);

        if (!res.compile())
            throw shader_error(res.get_info_log());

        return res;
    }

    shader compile_shader_file(GLenum mode, const std::string &file) {
        return compile_shader(mode, utilReadFile(file));
    }

    shaderprogram compile_shaderprogram(GLenum mode, const std::string &src) {
        shader sh = compile_shader(mode, src);

        shaderprogram res(mode);

        res.attach(sh);

        if (!res.link())
            throw shader_error(res.get_info_log());

        res.detach(sh);

        return res;
    }

    shaderprogram compile_shaderprogram_file(GLenum mode, const std::string &file) {
        return compile_shaderprogram(mode, utilReadFile(file));
    }
}
