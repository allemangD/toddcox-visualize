#pragma once

#include <tc/core.hpp>
#include <cmath>
#include <optional>
#include <numeric>
#include <iostream>
#include "combo_iterator.hpp"

template<int N>
using vec = Eigen::Matrix<float, N, 1>;
template<int N>
using mat = Eigen::Matrix<float, N, N>;

using vec1 = vec<1>;
using vec2 = vec<2>;
using vec3 = vec<3>;
using vec4 = vec<4>;
using vec5 = vec<5>;

using mat1 = mat<1>;
using mat2 = mat<2>;
using mat3 = mat<3>;
using mat4 = mat<4>;
using mat5 = mat<5>;

mat4 ortho(float left, float right, float bottom, float top, float front, float back) {
    mat<4> res = mat4();
    res <<
        2 / (right - left), 0, 0, -(right + left) / (right - left),
            0, 2 / (top - bottom), 0, -(top + bottom) / (top - bottom),
            0, 0, 2 / (front - back), -(front + back) / (front - back),
            0, 0, 0, 1;
    return res;
}

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

    explicit Primitive(const std::vector<unsigned> &values) {
        std::copy(values.begin(), values.begin() + N, inds.begin());
    }

    ~Primitive() = default;

    void apply(const tc::Cosets &table, int gen) {
        for (auto &ind : inds) {
            ind = table.get(ind, gen);
        }
    }
};
