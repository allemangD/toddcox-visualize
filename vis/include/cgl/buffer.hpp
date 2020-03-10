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

}
