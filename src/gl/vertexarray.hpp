#pragma once

#include <glad/glad.h>

#include <utility>

#include "buffer.hpp"
#include "types.hpp"

template<typename ...Fmt_>
class VertexArray {
public:
    template<size_t idx>
    using Fmt = std::tuple_element_t<idx, std::tuple<Fmt_...>>;

private:
    GLuint id = 0;

    template<size_t ...idx>
    inline void formatall(
        std::integer_sequence<size_t, idx...>
    ) {
        (AutoFormat<Fmt_>::Fmt::apply(id, idx), ...);
    }

    template<size_t ...idx>
    inline void bindall(
        const Binder<Fmt_> &...buf,
        std::integer_sequence<size_t, idx...>
    ) {
        (buf.bind(id, idx), ...);
    }

public:
    explicit VertexArray() {
        glCreateVertexArrays(1, &id);
        formatall(std::make_index_sequence<sizeof...(Fmt_)>());
    }

    explicit VertexArray(const Binder<Fmt_> &...buf)
        : VertexArray() {
        bind(buf...);
    }

    VertexArray(VertexArray &&o) noexcept {
        id = std::exchange(o.id, 0);
    }

    VertexArray(const VertexArray &) = delete;  // this is doable, but would be slow.

    ~VertexArray() {
        glDeleteVertexArrays(1, &id);
    }

    operator GLuint() const { // NOLINT(google-explicit-constructor)
        return id;
    }

    void bind(const Binder<Fmt_> &...buf) {
        bindall(buf..., std::make_index_sequence<sizeof...(Fmt_)>());
    }

    template<size_t idx>
    void bind(const Binder<Fmt<idx>> &buf) {
        buf.bind(id, idx);
    }
};
