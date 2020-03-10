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

        [[nodiscard]] size_t size() const {
            GLint res;
            glGetNamedBufferParameteriv(id, GL_BUFFER_SIZE, &res);
            return (size_t) res;
        }

        [[nodiscard]] size_t count() const {
            return size() / sizeof(T);
        }

        void put(const T &data, GLenum usage = GL_STATIC_DRAW) {
            glNamedBufferData(id, sizeof(T), &data, usage);
        }

        void put(const std::vector<T> &data, GLenum usage = GL_STATIC_DRAW) {
            glNamedBufferData(id, sizeof(T) * data.size(), &data[0], usage);
        }

        void bound(GLenum target, const std::function<void()> &action) const {
            glBindBuffer(target, id);
            action();
            glBindBuffer(target, 0);
        }
    };

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
                throw shader_error<mode>(get_info_log());
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

        template<GLenum mode>
        void attach(const shader<mode> &sh) {
            glAttachShader(id, sh);
        }

        template<GLenum mode>
        void detach(const shader<mode> &sh) {
            glDetachShader(id, sh);
        }

        bool link() {
            glLinkProgram(id);
            return (bool) get(GL_LINK_STATUS);
        }
    };

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
                throw shader_error<mode>(get_info_log());

            detach(sh);
        }

        static shaderprogram<mode> file(const std::string &name) {
            return shaderprogram<mode>(utilReadFile(name));
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

        pipeline &stage(const shaderprogram<GL_VERTEX_SHADER> &pgm) {
            glUseProgramStages(id, GL_VERTEX_SHADER_BIT, pgm);
            return *this;
        }

        pipeline &stage(const shaderprogram<GL_TESS_CONTROL_SHADER> &pgm) {
            glUseProgramStages(id, GL_TESS_CONTROL_SHADER_BIT, pgm);
            return *this;
        }

        pipeline &stage(const shaderprogram<GL_TESS_EVALUATION_SHADER> &pgm) {
            glUseProgramStages(id, GL_TESS_EVALUATION_SHADER_BIT, pgm);
            return *this;
        }

        pipeline &stage(const shaderprogram<GL_GEOMETRY_SHADER> &pgm) {
            glUseProgramStages(id, GL_GEOMETRY_SHADER_BIT, pgm);
            return *this;
        }

        pipeline &stage(const shaderprogram<GL_FRAGMENT_SHADER> &pgm) {
            glUseProgramStages(id, GL_FRAGMENT_SHADER_BIT, pgm);
            return *this;
        }

        pipeline &stage(const shaderprogram<GL_COMPUTE_SHADER> &pgm) {
            glUseProgramStages(id, GL_COMPUTE_SHADER_BIT, pgm);
            return *this;
        }

        void bound(const std::function<void()> &action) const {
            glBindProgramPipeline(id);
            action();
            glBindProgramPipeline(0);
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

    namespace pgm {
        using vert = shaderprogram<GL_VERTEX_SHADER>;
        using tcs = shaderprogram<GL_TESS_CONTROL_SHADER>;
        using tes = shaderprogram<GL_TESS_EVALUATION_SHADER>;
        using geom = shaderprogram<GL_GEOMETRY_SHADER>;
        using frag = shaderprogram<GL_FRAGMENT_SHADER>;
        using comp = shaderprogram<GL_COMPUTE_SHADER>;
    }

    class vertexarray {
        GLuint id{};

    public:
        vertexarray() {
            glCreateVertexArrays(1, &id);
        }

        vertexarray(vertexarray &) = delete;

        vertexarray(vertexarray &&o) noexcept {
            id = std::exchange(o.id, 0);
        }

        ~vertexarray() {
            glDeleteVertexArrays(1, &id);
            id = 0;
        }

        operator GLuint() const {
            return id;
        }

        void bound(const std::function<void()> &action) const {
            glBindVertexArray(id);
            action();
            glBindVertexArray(0);
        }
    };
}
