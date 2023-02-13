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
    using Color = Eigen::Vector<float, 3>;

    template<int R_, int D_, int G_>
    struct Structure {
        static constexpr auto Rank = R_;
        static constexpr auto Dim = D_;
        static constexpr auto Grade = G_;

        using AffineDf = Eigen::Transform<float, Dim, Eigen::Affine>;

        using VectorRf = Eigen::Vector<float, Rank>;

        // todo cache and recompute cells/points on frame (only if changed) in a system.

        tc::Group group;
        VectorRf root;

        AffineDf transform = AffineDf::Identity();

        explicit Structure(tc::Group const &group, VectorRf root) :
            group(group), root(root), transform(AffineDf::Identity()) {
        }
    };

    template<typename Str_>
    struct Part {
        using Str = Str_;

        entt::entity parent;

        GLuint first;
        GLuint count;
        Color color = Color::Ones();
        bool enabled = true;
    };

    struct Command {
        unsigned int count, instanceCount, first, baseInstance;
    };

    template<typename Str_>
    struct VBOs {
        using Str = Str_;

        using VectorDf = Str::VectorDf;
        using Color = Str::Color;
        using ArrayGui = Str::ArrayGui;

        struct Uniform {
            Eigen::Matrix4f linear;
            Eigen::Vector4f translation;
        };

        cgl::Buffer<VectorDf> vertices;
        cgl::Buffer<Color> colors;
        cgl::Buffer<ArrayGui> indices;
        cgl::Buffer<Uniform> uniform;
        cgl::Buffer<Command> commands;
    };

}
