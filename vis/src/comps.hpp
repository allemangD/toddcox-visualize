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
    struct Group {
        tc::Group group;
        vec5 root;
        vec3 color;

        std::vector<std::vector<size_t>> exclude {{0, 1, 2}};
        std::vector<std::vector<size_t>> include = combinations(_generators(group), 3);
    };

    struct VBOs {
        struct ModelMatrix {
            Eigen::Matrix4f linear;
            Eigen::Vector4f translation;
        };

        cgl::Buffer<vec4> verts;
        cgl::Buffer<Eigen::Array<unsigned int, 4, 1>> ibo;
        cgl::Buffer<ModelMatrix> ubo;

        using Affine4f = Eigen::Transform<float, 4, Eigen::Affine>;

        Affine4f tform = Affine4f::Identity();
    };

    void upload_groups(entt::registry &reg) {
        auto view = reg.view<const Group, VBOs>();

        for (auto [entity, group, vbos]: view.each()) {
            auto cosets = group.group.solve();
            auto mirrors = mirror<5>(group.group);
            auto corners = plane_intersections(mirrors);

            vec5 start = corners * group.root;

            tc::Path<vec5> path(cosets, mirrors.colwise());

            Eigen::Array<float, 5, Eigen::Dynamic> higher(5, path.order());
            path.walk(start, Reflect(), higher.matrix().colwise().begin());

            Eigen::Array<float, 4, Eigen::Dynamic> lower = Stereo()(higher);

            vbos.verts.put(lower.colwise().begin(), lower.colwise().end());

            // todo generate all, then mask using glMultiDraw.
            const size_t N = 4;

            auto inds = merge<N>(hull<N>(group.group, group.include, group.exclude));

            vbos.ibo.put(inds.colwise().begin(), inds.colwise().end());
        }
    }

    void upload_ubo(entt::registry &reg) {
        auto view = reg.view<VBOs>();

        for (auto [entity, vbos]: view.each()) {
            vbos.ubo.put({vbos.tform.linear(),
                          vbos.tform.translation()});
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
        }

        void operator()(entt::registry &reg) {
            auto view = reg.view<const Group, VBOs>();

            for (auto [entity, group, vbos]: view.each()) {
                const size_t N = 4;

                glBindProgramPipeline(pipe);

                glProgramUniform3fv(solid, 2, 1, group.color.data());
                glBindBufferBase(GL_UNIFORM_BUFFER, 2, vbos.ubo);

                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vbos.verts);

                vao.vertexBuffer(0, vbos.ibo);

                glBindVertexArray(vao);
                glDrawArrays(GL_POINTS, 0, vbos.ibo.count());
                glBindVertexArray(0);

                glBindProgramPipeline(0);
            }
        }
    };
}
