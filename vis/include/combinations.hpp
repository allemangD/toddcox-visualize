#pragma once

#include <vector>
#include <set>
#include <algorithm>

template<class V>
V select(const V &options, const std::vector<bool> &mask, size_t count) {
    V result;
    result.reserve(count);

    for (int i = 0; i < mask.size(); ++i) {
        if (mask[i]) result.push_back(options[i]);
    }

    return result;
}

template<class V>
std::set<V> combinations(const V &options, size_t count) {
    std::set<V> result;

    std::vector<bool> mask(options.size(), false);
    std::fill(mask.begin(), mask.begin() + count, true);

    do {
        result.insert(select(options, mask, count));
    } while (std::next_permutation(mask.begin(), mask.end(), std::greater<>()));

    return result;
}

template<class V>
std::set<V> difference(const std::set<V> &a, const std::set<V> &b) {
    std::set<V> result;

    std::set_difference(
        a.begin(), a.end(),
        b.begin(), b.end(),
        std::inserter(result, result.end())
    );

    return result;
}
