#pragma once

#include <tc/core.hpp>

#include <cmath>
#include <vector>
#include <algorithm>

#include <nanogui/glutil.h>

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

template<class V>
float dot(int n, const V &a, const V &b) {
    float sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

template<unsigned N>
std::vector<vec<N>> mirror(const tc::Group &group) {
    std::vector<std::vector<float>> mirrors;

    for (int p = 0; p < group.ngens; ++p) {
        std::vector<float> vp;
        for (int m = 0; m < p; ++m) {
            auto &vq = mirrors[m];
            vp.push_back((cos(M_PI / group.get(p, m)) - dot(m, vp, vq)) / vq[m]);
        }
        vp.push_back(std::sqrt(1 - dot(p, vp, vp)));

        for (const auto &v : mirrors) {
            if (dot(p, vp, vp) > 0) {
                for (auto &e : vp) {
                    e *= -1;
                }
                break;
            }
        }

        mirrors.push_back(vp);
    }

    std::vector<vec<N>> res;
    for (const auto &v : mirrors) {
        vec<N> rv = vec<N>::Zero();

        // ortho proj
        for (int i = 0; i < std::min(v.size(), (size_t) N); ++i) {
            rv[i] = v[i];
        }

        res.push_back(rv);
    }
    return res;
}

template<unsigned N>
vec<N> stereo(const vec<N + 1> &v) {
    vec<N> r;
    for (int i = 0; i < N; ++i) {
        r[i] = v[i] / (1 - v[N]);
    }
    return r;
}

template<unsigned N>
vec<N> ortho(const vec<N + 1> &v) {
    vec<N> r;
    for (int i = 0; i < N; ++i) {
        r[i] = v[i];
    }
    return r;
}

template<class V>
V project(const V &vec, const V &target) {
    return vec.dot(target) / target.dot(target) * target;
}

template<class V>
V reflect(const V &a, const V &axis) {
    return a - 2.f * project(a, axis);
}

template<class V>
V gram_schmidt_last(std::vector<V> vecs) {
    for (int i = 0; i < vecs.size(); ++i) {
        for (int j = 0; j < i; ++j) {
            vecs[i] -= project(vecs[i], vecs[j]);
        }
    }

    return vecs[vecs.size() - 1].normalized();
}

template<class V, class C>
V barycentric(const std::vector<V> &basis, const C &coords) {
    V res = V::Zero();

    int N = std::min((int) basis.size(), (int) coords.rows());
    for (int i = 0; i < N; ++i) {
        res += basis[i] * coords[i];
    }
    return res;
}

template<class V>
std::vector<V> plane_intersections(std::vector<V> normals) {
    std::vector<V> results(normals.size());

    for (int i = 0; i < normals.size(); ++i) {
        std::rotate(normals.begin(), normals.begin() + 1, normals.end());
        results[i] = gram_schmidt_last(normals);
    }

    return results;
}

template<unsigned N>
mat<N> rot(int u, int v, float theta) {
    mat<N> res = mat<N>::Identity();
    res(u, u) = std::cos(theta);
    res(u, v) = std::sin(theta);
    res(v, u) = -std::sin(theta);
    res(v, v) = std::cos(theta);
    return res;
}

mat4 ortho(float left, float right, float bottom, float top, float front, float back) {
    mat<4> res = mat4();
    res <<
        2 / (right - left), 0, 0, -(right + left) / (right - left),
        0, 2 / (top - bottom), 0, -(top + bottom) / (top - bottom),
        0, 0, 2 / (front - back), -(front + back) / (front - back),
        0, 0, 0, 1;
    return res;
}