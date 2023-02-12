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

        using VectorRf = Eigen::Vector<float, Rank>;
        using MatrixRXf = Eigen::Matrix<float, Rank, Eigen::Dynamic>;
        using VectorDf = Eigen::Vector<float, Dim>;
        using MatrixDXf = Eigen::Matrix<float, Dim, Eigen::Dynamic>;
        using VectorGf = Eigen::Vector<float, Grade>;
        using MatrixGXf = Eigen::Matrix<float, Grade, Eigen::Dynamic>;

        using ArrayRui = Eigen::Array<GLuint, Rank, 1>;
        using ArrayRXui = Eigen::Array<GLuint, Rank, Eigen::Dynamic>;
        using ArrayDui = Eigen::Array<GLuint, Dim, 1>;
        using ArrayDXui = Eigen::Array<GLuint, Dim, Eigen::Dynamic>;
        using ArrayGui = Eigen::Array<GLuint, Grade, 1>;
        using ArrayGXui = Eigen::Array<GLuint, Grade, Eigen::Dynamic>;

        using Color = Eigen::Vector<float, 3>;  // todo global typedef

        // todo cache and recompute cells/points on frame (only if changed) in a system.

        tc::Group group;
        VectorRf root;

        Points points;
        Hull<Grade> hull;

        std::vector<char> enabled;
        std::vector<Eigen::Vector3f> colors;

        Affine transform = Affine::Identity();

        explicit Structure(
            tc::Group group,
            VectorRf root,
            Color color_ = Color::Ones()
        ) :
            group(group),
            root(root),
            points(group, root),  // todo separate system
            hull(group),  // todo separate system
            enabled(hull.tilings.size(), true),
            colors(hull.tilings.size(), color_),
            transform(Affine::Identity()) {
        }
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

        struct Command {
            unsigned int count, instanceCount, first, baseInstance;
        };

        cgl::Buffer<VectorDf> vertices;
        cgl::Buffer<Color> colors;
        cgl::Buffer<ArrayGui> indices;
        cgl::Buffer<Uniform> uniform;
        cgl::Buffer<Command> commands;
    };

}
