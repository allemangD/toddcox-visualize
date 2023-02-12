#pragma once

#include <cgl/buffer.hpp>
#include <cgl/shaderprogram.hpp>
#include <cgl/vertexarray.hpp>
#include <cgl/pipeline.hpp>

#include <tc/groups.hpp>

#include <Eigen/Eigen>

#include <entt/entt.hpp>

#include "mirror.hpp"
#include "geometry.hpp"
#include "solver.hpp"

#include <shaders.hpp>

namespace vis {
    template<int R_, int D_, int G_>
    struct Structure {
        static constexpr auto Rank = R_;
        static constexpr auto Dim = D_;
        static constexpr auto Grade = G_;

        using Affine = Eigen::Transform<float, Dim, Eigen::Affine>;

        using Vertex = Eigen::Vector<float, Dim>;
        using Color = Eigen::Vector<float, 3>;
        using Cell = Eigen::Array<unsigned int, Grade, 1>;

        Points points;
        Hull <Grade> hull;

        std::vector<char> enabled;
        std::vector<Eigen::Vector3f> colors;

        Affine transform = Affine::Identity();

        template<typename P, typename H>
        explicit Structure(P &&points_, H &&hull_, Color color_ = Color::Ones()):
            points(std::forward<P>(points_)),
            hull(std::forward<H>(hull_)),
            enabled(hull.tilings.size(), true),
            colors(hull.tilings.size(), color_),
            transform(Affine::Identity()) {
        }
    };

    struct VBOs {
        struct Uniform {
            Eigen::Matrix4f linear;
            Eigen::Vector4f translation;
        };

        struct Command {
            unsigned int count, instanceCount, first, baseInstance;
        };

        cgl::Buffer<Structure<5, 4, 4>::Vertex> vertices;
        cgl::Buffer<Structure<5, 4, 4>::Color> colors;
        cgl::Buffer<Structure<5, 4, 4>::Cell> indices;
        cgl::Buffer<Uniform> uniform;
        cgl::Buffer<Command> commands;
    };

    void upload_structure(entt::registry &registry) {
        auto view = registry.view<Structure<5, 4, 4>, VBOs>();

        for (auto [entity, structure, vbos]: view.each()) {
            auto vertices = structure.points.verts.colwise();
            auto indices = structure.hull.inds.colwise();

            vbos.vertices.put(vertices.begin(), vertices.end());
            vbos.indices.put(indices.begin(), indices.end());
        }
    }

    void upload_uniforms(entt::registry &registry) {
        auto view = registry.view<Structure<5, 4, 4>, VBOs>();

        for (auto [entity, structure, vbos]: view.each()) {
            auto colors = structure.colors;
            VBOs::Uniform uniform{
                structure.transform.linear(),
                structure.transform.translation(),
            };

            vbos.colors.put(colors.begin(), colors.end());
            vbos.uniform.put(uniform, GL_STREAM_DRAW);
        }
    }

    void upload_commands(entt::registry &registry) {
        auto view = registry.view<Structure<5, 4, 4>, VBOs>();

        for (auto [entity, structure, vbos]: view.each()) {
            const auto &tilings = structure.hull.tilings;

            std::vector<VBOs::Command> commands;

            for (unsigned int i = 0; i < tilings.size(); ++i) {
                if (structure.enabled[i]) {
                    auto [first, count] = tilings[i];
                    commands.push_back({(unsigned int) count, 1, (unsigned int) first, i});
                }
            }

            vbos.commands.put(commands.begin(), commands.end(), GL_STREAM_DRAW);
        }
    }

    struct SliceRenderer {
        cgl::pgm::vert defer = cgl::pgm::vert(shaders::deferred_vs_glsl);
        cgl::pgm::geom slice = cgl::pgm::geom(shaders::slice_gm_glsl);
        cgl::pgm::frag solid = cgl::pgm::frag(shaders::solid_fs_glsl);

        cgl::pipeline pipe;

        cgl::VertexArray vao;

        SliceRenderer() {
            pipe.stage(defer);
            pipe.stage(slice);
            pipe.stage(solid);

            vao.iformat(0, 4, GL_UNSIGNED_INT);
            vao.format(1, 3, GL_FLOAT);

            glVertexArrayBindingDivisor(vao, 1, 1);
        }

        void operator()(entt::registry &reg) {
            auto view = reg.view<VBOs>();

            for (auto [entity, vbos]: view.each()) {
                const size_t N = 4;

                glBindProgramPipeline(pipe);

                glBindBufferBase(GL_UNIFORM_BUFFER, 2, vbos.uniform);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vbos.vertices);

                vao.vertexBuffer(0, vbos.indices);
                vao.vertexBuffer(1, vbos.colors);

                glBindBuffer(GL_DRAW_INDIRECT_BUFFER, vbos.commands);

                glBindVertexArray(vao);
                glMultiDrawArraysIndirect(GL_POINTS, nullptr, vbos.commands.count(), 0);
                glBindVertexArray(0);

                glBindProgramPipeline(0);
            }
        }
    };
}
