#pragma once

#include <memory>

#include <nanogui/opengl.h>

namespace cgl {
    template<class T>
    class Buffer {
        GLuint id{};

    public:
        Buffer() {
            glCreateBuffers(1, &id);
        }

        Buffer(const T &data, GLenum usage = GL_STATIC_DRAW)
            : Buffer() {
            put(data, usage);
        }

        Buffer(const std::vector<T> &data, GLenum usage = GL_STATIC_DRAW)
            : Buffer() {
            put(data, usage);
        }

        Buffer(const Buffer &) = delete;

        Buffer(Buffer &&o) noexcept {
            id = std::exchange(o.id, 0);
        };

        ~Buffer() {
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

        void put(const T *data, const size_t &size, GLenum usage = GL_STATIC_DRAW) {
            glNamedBufferData(id, sizeof(T) * size, data, usage);
        }

        template<class E>
        void put(const E &data, GLenum usage = GL_STATIC_DRAW) {
            put(data.data(), data.size(), usage);
        }

        void bound(GLenum target, const std::function<void()> &action) const {
            glBindBuffer(target, id);
            action();
            glBindBuffer(target, 0);
        }

        std::vector<T> getSubData(size_t offset, size_t count) const {
            GLintptr glOffset = offset * sizeof(T);
            GLsizeiptr glSize = count * sizeof(T);

            std::vector<T> data(count);

            glad_glGetNamedBufferSubData(id, glOffset, glSize, data.data());

            return data;
        }
    };

}
