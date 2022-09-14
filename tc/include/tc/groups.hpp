#pragma once

#include "core.hpp"


namespace tc {
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
