#pragma once

#include <array>
#include <vector>

namespace tc {
    struct Rel {
        std::array<int, 2> gens;
        int mult;
    };

    /**
     * A presentation of a coxeter group. Contains a number of generators and some relations of the form (ab)^n = e
     */
    struct Group {
        const int ngens;
        std::vector<std::vector<int>> _mults;  // lookup table for multiplicities
        std::string name;

        explicit Group(int ngens, const std::vector<Rel> &rels = {}, std::string name = "G");

        void setmult(Rel rel);

        [[nodiscard]] std::vector<Rel> get_rels() const;

        [[nodiscard]] Group product(const Group &other) const;

        [[nodiscard]] Group power(int p) const;
    };

    Group operator*(const Group &g, const Group &h);

    Group operator^(const Group &g, const int &p);

    /**
     * Construct a group from a (simplified) Schlafli Symbol of the form [a, b, ..., c]
     * @param mults: The sequence of multiplicites between adjacent generators.
     */
    Group schlafli(const std::vector<int> &mults);

    namespace group {
        Group A(int dim);

        Group B(int dim);

        Group D(int dim);

        Group E(int dim);

        Group F4();

        Group G2();

        Group H(int dim);

        Group I2(int n);

        Group T(int n, int m);

        Group T(int n);
    }
}