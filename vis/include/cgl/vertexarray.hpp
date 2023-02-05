#pragma once

#include <functional>
#include <string>
#include <utility>

#include <glad/glad.h>

#include <cgl/error.hpp>
#include <cgl/buffer.hpp>

namespace cgl {
    class VertexArray {
        GLuint id{};

    public:
        VertexArray() {
            glCreateVertexArrays(1, &id);
        }

        VertexArray(VertexArray &) = delete;

        VertexArray(VertexArray &&o) noexcept {
            id = std::exchange(o.id, 0);
        }

        ~VertexArray() {
            glDeleteVertexArrays(1, &id);
            id = 0;
        }

        operator GLuint() const {
            return id;
        }

        void format(
            GLuint index,
            unsigned size,
            GLenum type,
            bool normalized = false,
            unsigned stride = 0
        ) {
            glEnableVertexArrayAttrib(id, index);
            glVertexArrayAttribFormat(id, index, size, type, normalized, stride);
        }

        void iformat(
            GLuint index,
            unsigned size,
            GLenum type,
            unsigned stride = 0
        ) {
            glEnableVertexArrayAttrib(id, index);
            glVertexArrayAttribIFormat(id, index, size, type, stride);
        }

        template<class Buf>
        void vertexBuffer(
            GLuint index,
            Buf &buf,
            unsigned offset = 0
        ) {
            glVertexArrayVertexBuffer(id, index, buf, offset, sizeof(typename Buf::Element));
        }
    };
}