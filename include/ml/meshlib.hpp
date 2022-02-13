#pragma once

#include <iostream>
#include <ostream>
#include <memory>
#include <utility>

#include <Eigen/Eigen>

namespace ml {
    class MeshBase {
    public:
        using PointsType = Eigen::MatrixXf;
        using CellsType = Eigen::MatrixXi;

        [[nodiscard]] virtual PointsType points() const = 0;

        [[nodiscard]] virtual CellsType cells() const = 0;
    };

    class DynamicMesh : public MeshBase {
        PointsType _points;
        CellsType _cells;

    public:
        DynamicMesh() = default;

        explicit DynamicMesh(const MeshBase &mesh)
            : _points(mesh.points()), _cells(mesh.cells()) {}

        DynamicMesh(PointsType points, CellsType cells)
            : _points(std::move(points)), _cells(std::move(cells)) {}

        [[nodiscard]] PointsType &points() {
            return _points;
        }

        [[nodiscard]] PointsType points() const override {
            return _points;
        }

        [[nodiscard]] CellsType &cells() {
            return _cells;
        }

        [[nodiscard]] CellsType cells() const override {
            return _cells;
        }
    };

    class CubeMesh : public MeshBase {
        PointsType::Scalar radius;

    public:
        explicit CubeMesh(PointsType::Scalar radius = 0.5)
            : radius(radius) {}

        [[nodiscard]] PointsType points() const override {
            PointsType out(3, 8);
            out.transpose() <<
                +radius, +radius, +radius,
                +radius, +radius, -radius,
                +radius, -radius, +radius,
                +radius, -radius, -radius,
                -radius, +radius, +radius,
                -radius, +radius, -radius,
                -radius, -radius, +radius,
                -radius, -radius, -radius;
            return out;
        }

        CellsType cells() const override {
            CellsType out(3, 12);
            out.transpose() <<
                0b000, 0b001, 0b010, 0b001, 0b010, 0b011,
                0b100, 0b101, 0b110, 0b101, 0b110, 0b111,

                0b000, 0b001, 0b100, 0b001, 0b100, 0b101,
                0b010, 0b011, 0b110, 0b011, 0b110, 0b111,

                0b000, 0b010, 0b100, 0b010, 0b100, 0b110,
                0b001, 0b011, 0b101, 0b011, 0b101, 0b111;
            return out;
        }
    };
}
