# pragma once

#include <cgl/vertexarray.hpp>
#include <cgl/buffer.hpp>
#include <cgl/pipeline.hpp>

#include <geometry.hpp>
#include "mirror.hpp"

struct Matrices {
    mat4 proj = mat4::Identity();
    mat4 view = mat4::Identity();

    Matrices() = default;

    Matrices(mat4 proj, mat4 view) : proj(std::move(proj)), view(std::move(view)) {}

    static Matrices build(const nanogui::Screen &screen) {
        auto aspect = (float) screen.width() / (float) screen.height();
        auto pheight = 1.4f;
        auto pwidth = aspect * pheight;
//        auto proj = orthographic(-pwidth, pwidth, -pheight, pheight, -10.0f, 10.0f);
//        auto proj = perspective(-pwidth, pwidth, pheight, -pheight, 10.0f, 0.01f);
        auto proj = perspective(0.4, aspect, 0.1, 10.0);

        auto view = translation(0, 0, -4);

        return Matrices(proj, view);
    }
};

template<class T>
class Renderer {
public:
    virtual void draw(const T &prop) const = 0;
};

template<unsigned N>
class Slice {
private:
    const tc::Group group;

public:
    cgl::Buffer<unsigned> ibo;
    cgl::Buffer<vec4> vbo;
    cgl::VertexArray vao;

    explicit Slice(const tc::Group &g) : group(g) {
        auto gens = generators(group);
        auto combos = Combos<int>(gens, 3);
        std::vector<std::vector<int>> exclude = {{0, 1, 2}};

        auto all_sg_gens = combos;

        auto mesh = Mesh<N>::hull(g, generators(g), all_sg_gens);
        ibo.put(mesh);

        vao.ipointer(0, ibo, 4, GL_UNSIGNED_INT);
    }

    void setPoints(const vec5 &root, const mat5 &transform = mat5::Identity()) {
        auto cosets = group.solve();
        auto mirrors = mirror<5>(group);

        auto corners = plane_intersections(mirrors);
        auto start = barycentric(corners, root);

        auto higher = cosets.path.walk<vec5, vec5>(start, mirrors, reflect<vec5>);

        std::transform(
                higher.begin(), higher.end(), higher.begin(),
                [&](const vec5& v) { return transform * v; }
        );

        std::vector<vec4> lower(higher.size());
        std::transform(higher.begin(), higher.end(), lower.begin(), stereo<4>);

        vbo.put(lower);
    }
};

template<unsigned N>
class SliceRenderer : public Renderer<Slice<N>> {
private:
    cgl::pgm::vert defer = cgl::pgm::vert::file(
            "shaders/slice/deferred.vs.glsl");
    cgl::pgm::geom slice = cgl::pgm::geom::file(
            "shaders/slice/slice.gm.glsl");
    cgl::pgm::frag solid = cgl::pgm::frag::file(
            "shaders/solid.fs.glsl");

    cgl::pipeline pipe;

    cgl::Buffer<Matrices> ubo;

public:
    SliceRenderer() {
        pipe.stage(defer);
        pipe.stage(slice);
        pipe.stage(solid);
    }

    void draw(const Slice<N> &prop) const {
        glBindProgramPipeline(pipe);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, prop.vbo);
        glProgramUniform3f(solid, 2, 1.f, 1.f, 1.f);
        glBindVertexArray(prop.vao);
        glDrawArrays(GL_POINTS, 0, prop.ibo.count() * N);
    }
};
