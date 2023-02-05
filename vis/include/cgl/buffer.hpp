#pragma once

#include <memory>

#include <glad/glad.h>

namespace cgl {
    template<class T>
    class Buffer {
        GLuint id{};

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

        template<typename It>
        void put(It begin, It end, GLenum usage = GL_STATIC_DRAW) {
            glNamedBufferData(id, sizeof(T) * (end - begin), nullptr, usage);
            void* ptr = glMapNamedBuffer(id, GL_WRITE_ONLY);
            std::copy(begin, end, (T*) ptr);
            glUnmapNamedBuffer(id);
        }
    };
}
