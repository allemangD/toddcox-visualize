#pragma once

#include <tc/core.hpp>
#include <cmath>
#include <optional>
#include <numeric>
#include <iostream>
#include "combo_iterator.hpp"

template<unsigned N>
using Prims = Eigen::Matrix<unsigned, N, Eigen::Dynamic>;

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
