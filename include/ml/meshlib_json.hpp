#pragma once

#include "meshlib.hpp"

#include <Eigen/Eigen>
#include <nlohmann/json.hpp>

namespace Eigen {
    template<class Derived>
    void to_json(nlohmann::json &json, const Eigen::PlainObjectBase<Derived> &mat) {
        using Scalar = typename Derived::Scalar;

        auto rows = mat.rows();
        auto cols = mat.cols();

        std::vector<Scalar> vals(mat.size());
        Map<Derived>(vals.data(), rows, cols) = mat;

        json = {
            {"rows", rows},
            {"cols", cols},
            {"vals", vals},
        };
    }

    template<class Derived>
    void from_json(const nlohmann::json &j, Derived &mat) {
        using Scalar = typename Derived::Scalar;

        auto rows = j["rows"].get<Eigen::Index>();
        auto cols = j["cols"].get<Eigen::Index>();
        auto vals = j["vals"].get<std::vector<Scalar>>();

        mat = Eigen::Map<Derived>(vals.data(), rows, cols);
    }
}

namespace ml {
    static void to_json(nlohmann::json &json, const ml::MeshBase &mesh) {
        json = {
            {"points", mesh.points()},
            {"cells",  mesh.cells()},
        };
    }

    static void from_json(const nlohmann::json &j, ml::DynamicMesh &mesh) {
        mesh = {
            j["points"].get<DynamicMesh::PointsType>(),
            j["cells"].get<DynamicMesh::CellsType>(),
        };
    }

    void write(const ml::MeshBase &mesh, std::ostream &out) {
        nlohmann::json json = mesh;
        nlohmann::json::to_msgpack(json, out);
    }

    void write(const ml::MeshBase &mesh, const std::string &path) {
        std::ofstream file(path, std::ios::out | std::ios::binary);
        write(mesh, file);
    }

    ml::DynamicMesh read(std::istream &in) {
        nlohmann::json json = nlohmann::json::from_msgpack(in);
        return json.get<ml::DynamicMesh>();
    }

    ml::DynamicMesh read(const std::string &path) {
        std::ifstream file(path, std::ios::in | std::ios::binary);
        return read(file);
    }
}
