#pragma once

#include <tc/core.hpp>

#include <cmath>
#include <vector>
#include <algorithm>

#include <geo/geometry.hpp>

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
