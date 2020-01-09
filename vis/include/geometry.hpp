#pragma once

#include <tc/core.hpp>
#include <optional>
#include <cmath>

size_t key(const std::vector<int> &gens) {
    size_t bits = 0;
    for (const auto &gen : gens) {
        bits |= 1U << gen;
    }
    return bits;
}

struct CosetMemo {
    const tc::Group &parent;
    std::vector<std::vector<std::optional<tc::Cosets>>> results;

    explicit CosetMemo(const tc::Group &parent)
        : parent(parent) {
        size_t W = std::pow(2, parent.ngens);

        for (size_t i = 0; i < W; ++i) {
            results.emplace_back(W, std::nullopt);
        }
    }

    tc::Cosets solve(const std::vector<int> &group_gens, const std::vector<int> &sub_gens) {
        size_t group_key = key(group_gens);
        size_t sub_key = key(sub_gens);

        if (!results[group_key][sub_key]) {
            results[group_key][sub_key] = parent.subgroup(group_gens).solve(sub_gens);
        }

        return results[group_key][sub_key].value();
    }
};
