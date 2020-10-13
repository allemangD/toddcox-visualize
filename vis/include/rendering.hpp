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
        mat4 proj = ortho(-pwidth, pwidth, -pheight, pheight, -10.0f, 10.0f);

        auto view = mat4::Identity();
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

    template<class T>
    Slice(const tc::Group &g, T all_sg_gens, const std::vector<std::vector<int>> &exclude) : group(g) {
        const Prims<N> &data = merge<N>(hull<N>(g, all_sg_gens, exclude));
        ibo.put(data.data(), data.size());
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
