#pragma once

#include <tc/core.hpp>

#include <cmath>
#include <vector>
#include <algorithm>

#include <geometry.hpp>

template<unsigned int N>
Eigen::Matrix<float, N, N> mirror(const tc::Group<> &group) {
    Eigen::Matrix<float, N, N> res;
    res.setZero();

    for (int c = 0; c < group.rank(); ++c) {
        for (int r = 0; r < c; ++r) {
            auto angle = M_PI / group.get(c, r);
            auto dot = res.col(c).dot(res.col(r));

            res(r, c) = (cos(angle) - dot) / res(r, r);
        }

        res(c, c) = sqrt(1 - res.col(c).squaredNorm());
        res.col(c) *= -1;
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

template<class Point, class Axis>
auto project_(const Point &point, const Axis &axis) {
    return axis.dot(point) / axis.dot(axis) * axis;
}

template<class Mat>
Mat gram_schmidt(Mat mat) {
    for (int i = 0; i < mat.cols(); ++i) {
        for (int j = i + 1; j < mat.cols(); ++j) {
            mat.col(j) -= project_(mat.col(j), mat.col(i));
        }
    }
    return mat;
}

template<class Mat>
Mat plane_intersections(Mat normals) {
    auto last = normals.cols() - 1;

    Mat results(normals.rows(), normals.cols());
    results.setZero();

    Eigen::Matrix<int, Mat::ColsAtCompileTime, 1> indices(normals.cols());
    std::iota(indices.begin(), indices.end(), 0);

    for (int i = 0; i < normals.cols(); ++i) {
        std::rotate(indices.begin(), indices.begin() + 1, indices.end());

        Mat cur = normals * Eigen::PermutationMatrix<Mat::ColsAtCompileTime>(indices);
        Mat res = gram_schmidt(cur);

        results.col(i) = res.col(last);
    }

    results.colwise().normalize();

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
