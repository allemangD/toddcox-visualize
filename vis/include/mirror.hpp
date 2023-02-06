#pragma once

#include <tc/core.hpp>

#include <cmath>
#include <vector>
#include <algorithm>

#include <geometry.hpp>

template<unsigned int N>
Eigen::Matrix<float, N, N> mirror(const tc::Group &group) {
    Eigen::Matrix<float, N, N> res;
    res.setZero();

    for (int c = 0; c < group.rank(); ++c) {
        for (int r = 0; r < c; ++r) {
            auto angle = M_PI / group.get(c, r);
            auto dot = res.col(c).dot(res.col(r));

            res(r, c) = (dot - cos(angle)) / res(r, r);
        }

        res(c, c) = sqrt(1 - res.col(c).squaredNorm());
    }

    return res;
}

struct Stereo {
    template<class U>
    auto operator()(U &&mat) const {
        const auto Rows = std::remove_reference<U>::type::RowsAtCompileTime;
        return std::forward<U>(mat).template topRows<Rows - 1>().rowwise() / (1 - mat.template bottomRows<1>());
    }
};

struct Ortho {
    template<class U>
    auto operator()(U &&mat) const {
        const auto Rows = std::remove_reference<U>::type::RowsAtCompileTime;
        return std::forward<U>(mat).template topRows<Rows - 1>();
    }
};

struct Project {
    template<class U, class V>
    auto operator()(U &&point, V &&axis) const {
        return point.dot(axis) / axis.dot(axis) * std::forward<V>(axis);
    }
};

struct Reflect {
    template<class U, class V>
    auto operator()(U &&point, V &&axis) const {
        return std::forward<U>(point) - 2 * Project()(point, axis);
    }
};

template<class Mat>
Mat gram_schmidt(Mat mat) {
    for (int i = 0; i < mat.cols(); ++i) {
        for (int j = i + 1; j < mat.cols(); ++j) {
            mat.col(j) -= Project()(mat.col(j), mat.col(i));
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

template<class U, class V>
auto rotor(U const &u, V const &v) {
    using Rotor = Eigen::Matrix<
        typename U::Scalar,
        std::min(U::RowsAtCompileTime, V::RowsAtCompileTime),
        std::min(U::RowsAtCompileTime, V::RowsAtCompileTime),
        U::Options,
        std::max(U::MaxRowsAtCompileTime, V::MaxRowsAtCompileTime),
        std::max(U::MaxRowsAtCompileTime, V::MaxRowsAtCompileTime)
    >;

    const auto &ident = Rotor::Identity(u.rows(), v.rows());
    const auto &inner = (u + v) / (1 + u.dot(v)) * (u + v).transpose();
    const auto &outer = v * u.transpose();

    return ident - inner + 2 * outer;
}
