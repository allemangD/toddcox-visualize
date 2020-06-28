#pragma once

#include <tc/core.hpp>
#include <cmath>
#include <optional>
#include <numeric>
#include <iostream>
#include "combo_iterator.hpp"

/**
 * An primitive stage N indices.
 * @tparam N
 */
template<unsigned N>
struct Primitive {
    static_assert(N > 0, "Primitives must contain at least one point. Primitive<0> or lower is impossible.");

    std::array<unsigned, N> inds;

    Primitive() = default;

    Primitive(const Primitive<N> &) = default;

    Primitive(const Primitive<N - 1> &sub, unsigned root) {
        std::copy(sub.inds.begin(), sub.inds.end(), inds.begin());
        inds[N - 1] = root;
    }

    ~Primitive() = default;

    void apply(const tc::Cosets &table, int gen) {
        for (auto &ind : inds) {
            ind = table.get(ind, gen);
        }
    }
};
