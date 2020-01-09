#pragma once

#include <tc/core.hpp>
#include <glm/glm.hpp>
#include <cmath>
#include <vector>
#include <algorithm>

glm::vec4 round(const glm::vec4 &f, int prec) {
    auto dec = (float) pow(10, prec);
    auto res = glm::trunc(f * dec + 0.5f) / dec;
    return res;
}

float dot(int n, const glm::vec4 &a, const glm::vec4 &b) {
    float sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

std::vector<glm::vec4> mirror(const tc::Group &group) {
    std::vector<glm::vec4> mirrors;

    for (int p = 0; p < group.ngens; ++p) {
        glm::vec4 vp{};
        for (int m = 0; m < p; ++m) {
            glm::vec4 vq = mirrors[m];
            vp[m] = (cos(M_PI / group.get(p, m)) - dot(m, vp, vq)) / vq[m];
        }
        vp[p] = std::sqrt(1 - glm::dot(vp, vp));

        for (const auto &v : mirrors) {
            if (glm::dot(vp, v) > 0) {
                vp *= -1;
                break;
            }
        }

        mirrors.push_back(round(vp, 15));
    }

    return mirrors;
}

glm::vec4 project(const glm::vec4 &vec, const glm::vec4 &target) {
    return glm::dot(vec, target) / glm::dot(target, target) * target;
}

glm::vec4 reflect(const glm::vec4 &vec, const glm::vec4 axis) {
    return vec - 2.f * project(vec, axis);
}

glm::vec4 gram_schmidt_last(std::vector<glm::vec4> vecs) {
    int N = vecs.size();
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < i; ++j) {
            vecs[i] -= project(vecs[i], vecs[j]);
        }
    }

    return glm::normalize(vecs[N - 1]);
}

glm::vec4 barycentric(std::vector<glm::vec4> basis, std::vector<float> coords) {
    glm::vec4 res{};

    int N = std::min(basis.size(), coords.size());
    for (int i = 0; i < N; ++i) {
        res += basis[i] * coords[i];
    }
    return glm::normalize(res);
}

std::vector<glm::vec4> plane_intersections(std::vector<glm::vec4> normals) {
    int N = normals.size();
    std::vector<glm::vec4> results(N);

    for (int i = 0; i < N; ++i) {
        std::rotate(normals.begin(), normals.begin() + 1, normals.end());
        results[i] = gram_schmidt_last(normals);
    }

    return results;
}
