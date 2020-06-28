# pragma once

#include <cgl/vertexarray.hpp>
#include <cgl/buffer.hpp>
#include <cgl/pipeline.hpp>
#include <geometry.hpp>

#include <tuple>

template<unsigned N, class... T>
struct Prop {
    cgl::VertexArray vao;
    std::tuple<cgl::Buffer<T>...> vbos;
    cgl::Buffer<Primitive<N>> ibo;
};

template<unsigned N, class T>
struct Renderer {
    cgl::pipeline pipe;

    virtual void draw(const Prop<N, T> &prop) const = 0;
};

template<unsigned N, class T>
struct SliceRenderer : public Renderer<N, T> {
    cgl::pgm::vert defer = cgl::pgm::vert::file(
        "shaders/slice/deferred.vs.glsl");
    cgl::pgm::geom slice = cgl::pgm::geom::file(
        "shaders/slice/slice.gm.glsl");
    cgl::pgm::frag solid = cgl::pgm::frag::file(
        "shaders/solid.fs.glsl");

    cgl::pipeline pipe;

    SliceRenderer() {
        pipe.stage(defer);
        pipe.stage(slice);
        pipe.stage(solid);
    }

    void draw(const Prop<N, T> &prop) const override {
        pipe.bound([&]() {
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, std::get<0>(prop.vbos));
////        glProgramUniform3fv(solid, 2, 1, &prop.color.front());
            glProgramUniform3f(solid, 2, 1.f, 1.f, 1.f);
            prop.vao.bound([&]() {
                glDrawArrays(GL_POINTS, 0, prop.ibo.count() * N);
            });
        });
    }
};
