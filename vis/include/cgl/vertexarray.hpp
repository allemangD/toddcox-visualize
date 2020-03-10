#pragma once

#include <functional>
#include <string>
#include <utility>

#include <glad/glad.h>

#include <cgl/error.hpp>
#include <cgl/buffer.hpp>

namespace cgl {
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

        template<class T>
        void pointer(
            GLuint index,
            const buffer<T> &buf,
            unsigned size,
            GLenum type,
            bool normalized = false,
            unsigned stride = 0
        ) const {
            bound([&]() {
                glEnableVertexAttribArray(index);
                buf.bound(GL_ARRAY_BUFFER, [&]() {
                    glVertexAttribPointer(index, size, type, normalized, stride, nullptr);
                });
            });
        }

        template<class T>
        void ipointer(
            GLuint index,
            const buffer<T> &buf,
            unsigned size,
            GLenum type,
            unsigned stride = 0
        ) const {
            bound([&]() {
                glEnableVertexAttribArray(index);
                buf.bound(GL_ARRAY_BUFFER, [&]() {
                    glVertexAttribIPointer(index, size, type, stride, nullptr);
                });
            });
        }
    };
}