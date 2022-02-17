#pragma once

#include <glad/glad.h>

#include <Eigen/Eigen>

template<typename T_>
struct Buffer;

template<class ...Fmt_>
struct VertexArray;


template<typename ctype, GLenum type, GLint size, GLuint offset = 0, GLboolean norm = GL_FALSE>
struct Format {
    using CType = ctype;
    static constexpr GLenum Type = type;
    static constexpr GLint Size = size;
    static constexpr GLuint Offset = offset;
    static constexpr GLboolean Norm = norm;

    template<typename ctype_, GLint size_>
    using As = Format<ctype_, type, size_, offset, norm>;

    static inline constexpr void apply(GLuint vao, GLuint idx) {
        glEnableVertexArrayAttrib(vao, idx);
        glVertexArrayAttribFormat(vao, idx, size, type, norm, offset);
    }
};

template<typename ctype, GLenum type, GLint size, GLuint offset = 0>
struct IFormat {
    using CType = ctype;
    static constexpr GLenum Type = type;
    static constexpr GLint Size = size;
    static constexpr GLuint Offset = offset;

    template<typename ctype_, GLint size_>
    using As = IFormat<ctype_, type, size_, offset>;

    static inline constexpr void apply(GLuint vao, GLuint idx) {
        glEnableVertexArrayAttrib(vao, idx);
        glVertexArrayAttribIFormat(vao, idx, size, type, offset);
    }
};

template<typename ctype, GLenum type, GLint size, GLuint offset = 0>
struct LFormat {
    using CType = ctype;
    static constexpr GLenum Type = type;
    static constexpr GLint Size = size;
    static constexpr GLuint Offset = offset;

    template<typename ctype_, GLint size_>
    using As = LFormat<ctype_, type, size_, offset>;

    static inline constexpr void apply(GLuint vao, GLuint idx) {
        glEnableVertexArrayAttrib(vao, idx);
        glVertexArrayAttribLFormat(vao, idx, size, type, offset);
    }
};

template<typename Fmt_>
struct AutoFormat {
    using Fmt = Fmt_;
};

template<>
struct AutoFormat<GLdouble> {
    using Fmt = LFormat<GLdouble, GL_DOUBLE, 1>;
};

template<>
struct AutoFormat<GLfloat> {
    using Fmt = Format<GLfloat, GL_FLOAT, 1>;
};

template<>
struct AutoFormat<GLint> {
    using Fmt = IFormat<GLint, GL_INT, 1>;
};

template<>
struct AutoFormat<GLuint> {
    using Fmt = IFormat<GLuint, GL_UNSIGNED_INT, 1>;
};

template<typename Scalar, int Dim>
struct AutoFormat<Eigen::Vector<Scalar, Dim>> {
    static_assert((Dim >= 1) && (Dim <= 4), "Dim must be in range 1-4");

    using Fmt = typename AutoFormat<Scalar>::Fmt::template As<Eigen::Vector<Scalar, Dim>, Dim>;
};

template<typename Fmt_>
struct Binder {
    using CType = typename AutoFormat<Fmt_>::Fmt::CType;

    const GLuint buf;
    const GLuint offset;
    const GLuint stride;

    Binder(const Buffer<CType> &buf) // NOLINT(google-explicit-constructor)
        : buf((GLuint) buf), offset(0), stride(sizeof(CType)) {}

    template<typename CType_>
    Binder(const Buffer<CType_> &buf, GLuint offset)
        : buf((GLuint) buf), offset(offset), stride(sizeof(CType_)) {}

    Binder(const Binder<Fmt_> &o)
        : buf(o.buf), offset(o.offset), stride(o.stride) {}

    inline void bind(GLuint vao, GLuint idx) const {
        glVertexArrayVertexBuffer(vao, idx, buf, offset, stride);
    }
};

#define ATTR(buf, field) Binder<decltype(decltype(buf)::Type::field)>(buf, offsetof(decltype(buf)::Type, field))
