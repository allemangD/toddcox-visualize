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

        [[nodiscard]] Rel rel(int a, int b) const;

        [[nodiscard]] std::vector<Rel> get_rels() const;

        [[nodiscard]] Group product(const Group &other) const;

        [[nodiscard]] Group power(int p) const;

        [[nodiscard]] Group shrink(const std::vector<int> &gens) const;

        [[nodiscard]] bool trivial() const;
    };

    Group operator*(const Group &g, const Group &h);

    Group operator^(const Group &g, const int &p);

    /**
     * Construct a group from a (simplified) Schlafli Symbol of the form [a, b, ..., c]
     * @param mults: The sequence of multiplicites between adjacent generators.
     */
    Group schlafli(const std::vector<int> &mults);

    namespace group {
        /**
         * Simplex
         */
        Group A(int dim);

        /**
         * Cube, Orthoplex
         */
        Group B(int dim);

        /**
         * Demicube, Orthoplex
         */
        Group D(int dim);

        /**
         * E groups
         */
        Group E(int dim);

        /**
         * 24 Cell
         */
        Group F4();

        /**
         * Hexagon
         */
        Group G2();

        /**
         * Icosahedron
         */
        Group H(int dim);

        /**
         * Polygonal
         */
        Group I2(int n);

        /**
         * Toroidal. I2(n) * I2(m)
         */
        Group T(int n, int m);

        /**
         * Toroidal. T(n, n)
         */
        Group T(int n);
    }
}