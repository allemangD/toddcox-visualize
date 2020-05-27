# pragma once

#include <cgl/vertexarray.hpp>
#include <cgl/buffer.hpp>
#include <cgl/pipeline.hpp>
#include <geometry.hpp>

template<unsigned N, class T>
struct Prop {
    cgl::VertexArray vao;
    cgl::Buffer<T> vbo;
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
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, prop.vbo);
////        glProgramUniform3fv(solid, 2, 1, &prop.color.front());
            glProgramUniform3f(solid, 2, 1.f, 1.f, 1.f);
            prop.vao.bound([&]() {
                glDrawArrays(GL_POINTS, 0, prop.ibo.count() * N);
            });
        });
    }
};

//template<unsigned N, class T>
//struct Prop {
//    cgl::VertexArray vao;
//    cgl::Buffer<T> vbo;
//    cgl::Buffer<Primitive<N>> ibo;
//
//    Prop() : vao(), vbo(), ibo() {}
//};
//
//template<unsigned N, class T>
//struct Renderer {
//    std::vector<Prop<N, T>> props;
//
//    virtual void bound(const std::function<void()> &action) const = 0;
//
//    virtual void _draw(const Prop<N, T> &) const = 0;
//
//    void render() const {
//        bound([&]() {
//            for (const auto &prop : props) {
//                _draw(prop);
//            }
//        });
//    }
//};
//
//template<unsigned N, class T>
//struct SliceProp : public Prop<N, T> {
//    vec3 color;
//
//    SliceProp(vec3 color) : Prop<N, T>(), color(color) {}
//
//    SliceProp(SliceProp &) = delete;
//
//    SliceProp(SliceProp &&) noexcept = default;
//
//    template<class T, class C>
//    static SliceProp<N> build(
//        const tc::Group &g,
//        const C &coords,
//        vec3 color,
//        T all_sg_gens,
//        const std::vector<std::vector<int>> &exclude
//    ) {
//        SliceProp<N> res(color);
//
//        res.vbo.put(points(g, coords));
//        res.ibo.put(merge<N>(hull<N>(g, all_sg_gens, exclude)));
//        res.vao.ipointer(0, res.ibo, 4, GL_UNSIGNED_INT);
//
//        return res;
//    }
//};
//
//template<unsigned N>
//struct SliceRenderer : public Renderer<N> {
//    cgl::pgm::vert defer = cgl::pgm::vert::file(
//        "shaders/slice/deferred.vs.glsl");
//    cgl::pgm::geom slice = cgl::pgm::geom::file(
//        "shaders/slice/slice.gm.glsl");
//    cgl::pgm::frag solid = cgl::pgm::frag::file(
//        "shaders/solid.fs.glsl");
//
//    cgl::pipeline pipe;
//
//    SliceRenderer() {
//        pipe.stage(defer);
//        pipe.stage(slice);
//        pipe.stage(solid);
//    }
//
//    SliceRenderer(SliceRenderer &) = delete;
//
//    SliceRenderer(SliceRenderer &&) noexcept = default;
//
//    void bound(const std::function<void()> &action) const override {
//        pipe.bound(action);
//    }
//
//    void _draw(const Prop<N> &prop) const override {
//        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, prop.vbo);
////        glProgramUniform3fv(solid, 2, 1, &prop.color.front());
//        glProgramUniform3f(solid, 2, 1.f, 1.f, 1.f);
//        prop.vao.bound([&]() {
//            glDrawArrays(GL_POINTS, 0, prop.ibo.count() * N);
//        });
//    }
//};
//
//
//template<unsigned N>
//struct DirectRenderer : public Renderer<N> {
//    cgl::pipeline pipe;
//
//    DirectRenderer() = default;
//
//    DirectRenderer(DirectRenderer &) = delete;
//
//    DirectRenderer(DirectRenderer &&) noexcept = default;
//
//    void bound(const std::function<void()> &action) const override {
//        pipe.bound(action);
//    }
//
//    void _draw(const Prop<N> &prop) const override {
//        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, prop.vbo);
////        glProgramUniform3fv(sh.solid, 2, 1, &wire.color.front());
//        prop.vao.bound([&]() {
//            prop.ibo.bound(GL_ELEMENT_ARRAY_BUFFER, [&]() {
//                glDrawElements(GL_LINES, prop.ibo.count() * N, GL_UNSIGNED_INT, nullptr);
//            });
//        });
//    }
//};
//
//struct WireframeProp : public Prop<2> {
//    vec3 color;
//
//    WireframeProp(vec3 color) : Prop<2>(), color(color) {}
//
//    WireframeProp(WireframeProp &) = delete;
//
//    WireframeProp(WireframeProp &&) noexcept = default;
//
//    template<class T, class C>
//    static WireframeProp build(const tc::Group &g,
//        const C &coords,
//        bool curve,
//        bool ortho,
//        vec3 color,
//        T all_sg_gens,
//        const std::vector<std::vector<int>> &exclude
//    ) {
//        WireframeProp res(color);
//
//        res.vbo.put(points(g, coords));
//        res.ibo.put(merge<2>(hull<2>(g, all_sg_gens, exclude)));
//
//        return res;
//    }
//};
