#pragma once

#include <vector>
#include <array>
#include <algorithm>
#include <numeric>
#include <stdexcept>

size_t choose(size_t n, size_t k) {
    if (k == 0) return 1;
    return n * choose(n - 1, k - 1) / k;
}

template<class T>
class ComboIterator {
private:
    const std::vector<T> &options;

    std::vector<bool> bits;
    std::vector<T> curr;
    int at;

    void set_curr() {
        for (int i = 0, j = 0; i < bits.size(); ++i) {
            if (bits[i]) curr[j++] = options[i];
        }
    }

public:
    ComboIterator(const std::vector<T> &options, int k, int at = 0)
        : options(options), bits(options.size()), curr(k), at(at) {
        std::fill(bits.begin(), bits.begin() + k, true);
        set_curr();
    }

    [[nodiscard]] bool operator==(const ComboIterator<T> &o) const {
        return at == o.at;
    }

    [[nodiscard]] bool operator!=(const ComboIterator<T> &o) const {
        return at != o.at;
    }

    auto operator*() const {
        return curr;
    }

    const auto &operator->() const {
        return &this;
    }

    auto operator++(int) {
        std::prev_permutation(bits.begin(), bits.end());
        set_curr();
        ++at;
        return *this;
    }

    auto operator++() &{
        auto res = *this;
        (*this)++;
        return res;
    }

    auto operator--(int) {
        std::next_permutation(bits.begin(), bits.end());
        set_curr();
        --at;
        return *this;
    }

    auto operator--() &{
        auto res = *this;
        (*this)--;
        return res;
    }
};

template<class T>
class Combos {
private:
    const std::vector<T> options;
    int k;
    int size;

public:
    Combos(const std::vector<T> &options, int k)
        : options(options), k(k), size(choose(options.size(), k)) {
    }

    ComboIterator<T> begin() const {
        return ComboIterator<T>(options, k);
    }

    ComboIterator<T> end() const {
        return ComboIterator<T>(options, k, size);
    }
};
