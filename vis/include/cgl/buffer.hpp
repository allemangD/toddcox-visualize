#pragma once

#include <memory>

#include <glad/glad.h>

namespace cgl {
    template<class T>
    class Buffer {
        GLuint id{};
        size_t _count = 0;

    public:
        using Element = T;

        Buffer() {
            glCreateBuffers(1, &id);
        }

        Buffer(Buffer &) = delete;

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
            return _count * sizeof(T);
        }

        [[nodiscard]] size_t count() const {
            return _count;
        }

        template<typename It>
        void put(It begin, It end, GLenum usage = GL_STATIC_DRAW) {
            _count = end - begin;

            glNamedBufferData(id, sizeof(T) * _count, nullptr, usage);
            void* ptr = glMapNamedBuffer(id, GL_WRITE_ONLY);
            std::copy(begin, end, (T*) ptr);
            glUnmapNamedBuffer(id);
        }

        void put(const T &data, GLenum usage = GL_STATIC_DRAW) {
            T const* ptr = &data;
            put(ptr, ptr + 1, usage);
        }
    };
}
