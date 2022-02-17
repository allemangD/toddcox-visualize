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
    void from_json(const nlohmann::json &j, Derived &d) {
        using Scalar = typename Derived::Scalar;

        auto rows = j["rows"].get<Index>();
        auto cols = j["cols"].get<Index>();
        auto vals = j["vals"].get<std::vector<Scalar>>();

        d = Map<Derived>(vals.data(), rows, cols);
    }
}

namespace nlohmann {
    template<typename PT_, typename CT_>
    struct adl_serializer<ml::Mesh<PT_, CT_>> {
        static void to_json(json &j, const ml::Mesh<PT_, CT_> &m) {
            j = {
                {"points", m.points},
                {"cells",  m.cells},
            };
        }

        static ml::Mesh<PT_, CT_> from_json(const json &j) {
            return ml::Mesh<PT_, CT_>(
                j["points"].get<PT_>(),
                j["cells"].get<CT_>()
            );
        }
    };
}

namespace ml {
    template<typename PT_, typename CT_>
    void write(const ml::Mesh<PT_, CT_> &mesh, std::ostream &&out) {
        nlohmann::json json = mesh;
        nlohmann::json::to_msgpack(json, out);
    }

    template<typename M_>
    M_ read(std::istream &&in) {
        return nlohmann::json::from_msgpack(in).get<M_>();
    }
}
