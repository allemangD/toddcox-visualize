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

        [[nodiscard]] virtual Eigen::Index dim() const = 0;

        [[nodiscard]] virtual PointsType points() const = 0;

        [[nodiscard]] virtual Eigen::Index rank() const = 0;

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

        Eigen::Index dim() const override {
            return _points.rows();
        }

        [[nodiscard]] PointsType &points() {
            return _points;
        }

        [[nodiscard]] PointsType points() const override {
            return _points;
        }

        Eigen::Index rank() const override {
            return _points.rows();
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

        Eigen::Index dim() const override {
            return 3;
        }

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

        Eigen::Index rank() const override {
            return 3;
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

    class WireCubeMesh : public MeshBase {
        PointsType::Scalar radius;
        long _dim;
        long _npoints;
        long _nlines;

    public:
        explicit WireCubeMesh(long dim = 3, PointsType::Scalar radius = 0.5)
            : _dim(dim), radius(radius) {
            _npoints = 1 << _dim;
            _nlines = _dim * (_npoints >> 1);
        }

        Eigen::Index dim() const override {
            return _dim;
        }

        [[nodiscard]] PointsType points() const override {
            PointsType out(_dim, _npoints);
            out.fill(radius);
            for (int i = 0; i < out.cols(); ++i) {
                for (int j = 0; j < _dim; ++j) {
                    if ((i >> j) & 1) {
                        out(j, i) *= -1;
                    }
                }
            }
            return out;
        }

        Eigen::Index rank() const override {
            return 2;
        }

        [[nodiscard]] CellsType cells() const override {
            CellsType out(2, _nlines);
            int k = 0;
            for (int i = 0; i < _npoints; ++i) {
                for (int j = 0; j < _dim; ++j) {
                    if ((i >> j) & 1) {
                        out(0, k) = i;
                        out(1, k) = i ^ (1 << j);
                        k++;
                    }
                }
            }
            return out;
        }
    };
}
