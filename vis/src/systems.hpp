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
#include "components.hpp"

#include <shaders.hpp>

namespace vis {
    template<typename Str>
    void upload_structure(entt::registry &registry) {
        {
            auto parts = registry.view<Part<Str>>();
            registry.destroy(parts.begin(), parts.end());
        }

        auto view = registry.view<Str, VBOs<Str>>();

        for (auto [entity, structure, vbos]: view.each()) {
            Points points(structure.group, structure.root);
            Hull<Str::Grade> hull(structure.group);

            auto &&vertices = points.verts.colwise();
            auto &&indices = hull.inds.colwise();

            vbos.vertices.put(vertices.begin(), vertices.end());
            vbos.indices.put(indices.begin(), indices.end());

            for (const auto &tiling: hull.tilings) {
                auto part_entity = registry.create();
                registry.emplace<Part<Str>>(
                    part_entity,
                    entity,
                    tiling.first,
                    tiling.count
                );
            }
        }
    }

    template<typename Str>
    void upload_uniforms(entt::registry &registry) {
        auto view = registry.view<Part<Str>>();

        for (auto [entity, part]: view.each()) {
            auto &vbos = registry.get<VBOs<Str>>(part.parent);
        }

        for (auto [entity, structure, vbos]: view.each()) {
            std::vector<typename Str::Color> colors;
            for (const auto &part_entity: structure.parts) {
                const auto &part = registry.get<typename Str::Part>(part_entity);
                colors.push_back(part.color);
            }
            vbos.colors.put(colors.begin(), colors.end());

            typename VBOs<Str>::Uniform uniform{
                structure.transform.linear(),
                structure.transform.translation(),
            };
            vbos.uniform.put(uniform, GL_STREAM_DRAW);
        }
    }

    template<typename Str>
    void upload_commands(entt::registry &registry) {
        auto view = registry.view<Part<Str>>();
        for (auto [entity, part]: view.each()) {
            Command comm(part.count, 1, part.first, )
        }

        auto view = registry.view<Str, VBOs<Str>>();

        for (auto [entity, structure, vbos]: view.each()) {
            std::vector<typename VBOs<Str>::Command> commands;

            for (unsigned int idx = 0; idx < structure.parts.size(); idx++) {
                const auto &part_entity = structure.parts[idx];
                const auto &part = registry.get<typename Str::Part>(part_entity);
                if (part.enabled) {
                    commands.push_back(
                        {
                            part.count,
                            1,
                            part.first,
                            idx
                        }
                    );
                }
            }

            vbos.commands.put(commands.begin(), commands.end(), GL_STREAM_DRAW);
        }
    }

    template<typename Str_>
    struct SliceRenderer {
        using Str = Str_;

        cgl::pgm::vert defer = cgl::pgm::vert(shaders::deferred_vs_glsl);
        cgl::pgm::geom slice = cgl::pgm::geom(shaders::slice_gm_glsl);
        cgl::pgm::frag solid = cgl::pgm::frag(shaders::solid_fs_glsl);

        cgl::pipeline pipe;

        cgl::VertexArray vao;

        SliceRenderer() {
            pipe.stage(defer);
            pipe.stage(slice);
            pipe.stage(solid);

            vao.iformat(0, Str::Grade, GL_UNSIGNED_INT);  // inds

            vao.format(1, 3, GL_FLOAT);  // color
            glVertexArrayBindingDivisor(vao, 1, 1);
        }

        void operator()(entt::registry &reg) {
            auto view = reg.view<VBOs<Str>>();

            for (auto [entity, vbos]: view.each()) {
                glBindProgramPipeline(pipe);

                glBindBufferBase(
                    GL_SHADER_STORAGE_BUFFER,
                    1,
                    vbos.vertices
                );
                glBindBufferBase(
                    GL_UNIFORM_BUFFER,
                    2,
                    vbos.uniform
                );

                glBindBuffer(
                    GL_DRAW_INDIRECT_BUFFER,
                    vbos.commands
                );

                vao.vertexBuffer(0, vbos.indices);
                vao.vertexBuffer(1, vbos.colors);

                glBindVertexArray(vao);
                glMultiDrawArraysIndirect(
                    GL_POINTS,
                    nullptr,
                    vbos.commands.count(),
                    0
                );
                glBindVertexArray(0);

                glBindProgramPipeline(0);
            }
        }
    };
}