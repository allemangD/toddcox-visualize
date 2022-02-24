#pragma once

#include <set>
#include <algorithm>

template<typename V, typename M>
V select(const V &data, const M &mask, size_t count) {
    V result;
    result.reserve(count);

    for (int i = 0; i < mask.size(); ++i) {
        if (mask[i]) result.push_back(data[i]);
    }

    return result;
}

template<typename V>
std::vector<V> combinations(const V &data, const size_t count) {
    std::vector<V> result;

    std::vector<bool> mask(data.size(), false);
    std::fill(mask.begin(), mask.begin() + count, true);

    do {
        result.push_back(select(data, mask, count));
    } while (std::next_permutation(mask.begin(), mask.end(), std::greater<>()));

    return result;
}
