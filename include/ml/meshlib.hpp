#pragma once

#include <iostream>
#include <ostream>
#include <memory>
#include <utility>

#include <Eigen/Eigen>

namespace ml {
    using Matrix2Xui = Eigen::Matrix<unsigned int, 2, Eigen::Dynamic>;
    using Matrix3Xui = Eigen::Matrix<unsigned int, 3, Eigen::Dynamic>;
    using Matrix4Xui = Eigen::Matrix<unsigned int, 4, Eigen::Dynamic>;

    template<typename PT_, typename CT_>
    class Mesh {
    public:
        using Points = PT_;
        using Cells = CT_;

        Points points;
        Cells cells;

        Mesh(Points points, Cells cells)
            : points(std::move(points)), cells(std::move(cells)) {}
    };

    auto make_cube(float radius) {
        Eigen::Matrix3Xf points(3, 8);
        points.fill(radius);
        for (int i = 0; i < points.cols(); ++i) {
            for (int j = 0; j < 3; ++j) {
                if ((i >> j) & 1) {
                    points(j, i) *= -1;
                }
            }
        }

        Matrix3Xui cells(3, 12);
        cells.transpose()
            << 0b000, 0b001, 0b010, 0b001, 0b010, 0b011,
            0b100, 0b101, 0b110, 0b101, 0b110, 0b111,

            0b000, 0b001, 0b100, 0b001, 0b100, 0b101,
            0b010, 0b011, 0b110, 0b011, 0b110, 0b111,

            0b000, 0b010, 0b100, 0b010, 0b100, 0b110,
            0b001, 0b011, 0b101, 0b011, 0b101, 0b111;

        return Mesh(points, cells);
    }

    template<size_t Dim>
    auto make_cube_wire(float radius) {
        constexpr size_t NPoints = 1 << Dim;
        constexpr size_t NCells = Dim * (NPoints >> 1);

        Eigen::Matrix<float, Dim, NPoints> points;
        points.fill(radius);
        for (int i = 0; i < points.cols(); ++i) {
            for (int j = 0; j < Dim; ++j) {
                if ((i >> j) & 1) {
                    points(j, i) *= -1;
                }
            }
        }

        Eigen::Matrix<unsigned int, 2, NCells> cells;
        int k = 0;
        for (int i = 0; i < NPoints; ++i) {
            for (int j = 0; j < Dim; ++j) {
                if ((i >> j) & 1) {
                    cells(0, k) = i;
                    cells(1, k) = i ^ (1 << j);
                    k++;
                }
            }
        }

        return Mesh(points, cells);
    }
}
