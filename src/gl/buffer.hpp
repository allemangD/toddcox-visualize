#pragma once

#include <glad/glad.h>

#include <utility>

template<typename T_>
class Buffer {
public:
    using Type = T_;

private:
    GLuint id = 0;

public:
    Buffer() {
        glCreateBuffers(1, &id);
    }

    Buffer(Buffer&& o) noexcept {
        id = std::exchange(o.id, 0);
    }

    Buffer(const Buffer&) = delete;  // this is doable, but would be slow.

    operator GLuint() const { // NOLINT(google-explicit-constructor)
        return id;
    }

    template<typename RandomIt>
    GLuint upload(RandomIt first, RandomIt last, GLenum mode = GL_STATIC_DRAW) {
        size_t count = last - first;

        // todo StaticBuffer that uses BufferStorage
        glNamedBufferData(id, sizeof(Type) * count, nullptr, mode);

        Type* out = (Type*) glMapNamedBuffer(id, GL_WRITE_ONLY);
        std::copy(first, last, out);
        glUnmapNamedBuffer(id);

        return count;
    }

    template<typename T>
    GLuint upload(const T& data, GLenum mode = GL_STATIC_DRAW) {
        return upload(data.begin(), data.end(), mode);
    }

    ~Buffer() {
        // delete silently ignores 0.
        glDeleteBuffers(1, &id);
    }
};
