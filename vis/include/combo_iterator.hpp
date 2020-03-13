#pragma once

#include <array>
#include <algorithm>
#include <numeric>
#include <stdexcept>

template<size_t N, size_t K>
class ComboIterator {
    static_assert(N > K, "N must be larger than K");

public:
    size_t at;
    std::array<bool, N> bits;
    std::array<size_t, K> curr;

    void inc() {
        std::prev_permutation(bits.begin(), bits.end());
        for (int i = 0, k = 0; i < N; ++i) {
            if (bits[i]) curr[k++] = i;
        }
        at++;
    }

public:
    ComboIterator(size_t at = 0) : at(at), bits(), curr() {
        std::iota(curr.begin(), curr.end(), 0);
        std::fill(bits.begin(), bits.begin() + K, true);
    }

    [[nodiscard]] bool operator==(const ComboIterator<N, K> &o) const {
        return at == o.at;
    }

    [[nodiscard]] bool operator!=(const ComboIterator<N, K> &o) const {
        return at != o.at;
    }

    auto operator*() const {
        return curr;
    }

    const auto *operator->() const {
        return &curr;
    }

    auto operator++(int) {
        auto res = *this;
        inc();
        return res;
    }

    auto operator++() &{
        inc();
        return *this;
    }
};

size_t choose(size_t n, size_t k) {
    if (k == 0) return 1;
    return n * choose(n - 1, k - 1) / k;
}

template<size_t N, size_t K>
class Combos {
private:
public:
    auto begin() const {
        return ComboIterator<N, K>();
    }

    auto end() const {
        return ComboIterator<N, K>(choose(N, K));
    }
};
