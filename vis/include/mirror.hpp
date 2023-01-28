#pragma once

#include <tc/core.hpp>

#include <cmath>
#include <vector>
#include <algorithm>

#include <Eigen/Eigen>

template<unsigned N>
using vec = std::array<float, N>;

using vec1 = vec<1>;
using vec2 = vec<2>;
using vec3 = vec<3>;
using vec4 = vec<4>;
using vec5 = vec<5>;

template<class V>
V operator*(V a, const float &b) {
    for (auto &e : a) e *= b;
    return a;
}

template<class V>
V operator*(const float &b, V a) {
    for (auto &e : a) e *= b;
    return a;
}

template<class V>
V operator/(V a, const float &b) {
    for (auto &e : a) e /= b;
    return a;
}

template<class V>
V operator+(const V &a, V b) {
    for (int i = 0; i < a.size(); ++i) {
        a[i] += b[i];
    }
    return a;
}

template<class V>
V operator-(V a, const V &b) {
    for (int i = 0; i < a.size(); ++i) {
        a[i] -= b[i];
    }
    return a;
}

template<class V>
void operator-=(V &a, const V &b) {
    for (int i = 0; i < a.size(); ++i) {
        a[i] -= b[i];
    }
}

template<class V>
void operator+=(V &a, const V &b) {
    for (int i = 0; i < a.size(); ++i) {
        a[i] += b[i];
    }
}

template<class V>
float length(const V &a) {
    float sum = 0;
    for (const auto &e : a) sum += e * e;
    return sqrtf(sum);
}

template<class V>
V normalized(const V &a) {
    return a / length(a);
}

template<class V>
float dot(int n, const V &a, const V &b) {
    float sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

template<class V>
float dot(const V &a, const V &b) {
    float sum = 0;
    for (int i = 0; i < a.size(); ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

template<unsigned N>
std::vector<vec<N>> mirror(const tc::Group<> &group) {
    std::vector<std::vector<float>> mirrors;

    for (int p = 0; p < group.rank(); ++p) {
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
        vec<N> rv{};

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

template<class V>
V project(const V &vec, const V &target) {
    return dot(vec, target) / dot(target, target) * target;
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

    return normalized(vecs[vecs.size() - 1]);
}

template<class V, class C>
V barycentric(const std::vector<V> &basis, const C &coords) {
    V res{};

    int N = std::min(basis.size(), coords.size());
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

Eigen::Matrix4f utilRotate(const int u, const int v, const float theta) {
    Eigen::Matrix4f res;
    res.setIdentity();
    res(u, u) = std::cos(theta);
    res(u, v) = std::sin(theta);
    res(v, u) = -std::sin(theta);
    res(v, v) = std::cos(theta);
    return res;
}

Eigen::Matrix4f ortho(
    float l,
    float r,
    float b,
    float t,
    float n,
    float f
) {
    Eigen::Matrix4f res;
    res <<
        2 / (r - l), 0, 0, -(r + l) / (r - l),
        0, 2 / (t - b), 0, -(t + b) / (t - b),
        0, 0, -2 / (f - n), -(f + n) / (f - n),
        0, 0, 0, 1;
    return res;
}
