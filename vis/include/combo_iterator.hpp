#pragma once

#include <vector>
#include <algorithm>
#include <stdexcept>

template<class T>
struct ComboIterator {
    typedef ComboIterator<T> self_type;
    typedef const std::vector<T> value_type;;
    typedef const std::vector<T> *pointer;
    typedef const std::vector<T> &reference;
    typedef size_t difference_type;
    typedef std::forward_iterator_tag iterator_category;

    const std::vector<T> &vals;
    const size_t k;
    size_t n;
    std::vector<bool> bits;
    std::vector<T> curr;

    ComboIterator(ComboIterator &) = default;

    ComboIterator(const std::vector<T> &vals, const size_t k, const size_t n)
        : vals(vals), k(k), n(n), curr(k), bits(vals.size()) {
        for (size_t i = 0; i < vals.size(); ++i) {
            bits[i] = i < k;
        }
        std::reverse(bits.begin(), bits.end());
        set_curr();
    }

    ~ComboIterator() = default;

    void set_curr() {
        for (size_t i = 0, j = 0; i < vals.size(); ++i) {
            if (bits[i]) curr[j++] = vals[i];
        }
    }

    [[nodiscard]] bool operator==(const ComboIterator<T> &o) const {
        return n == o.n;
    }

    [[nodiscard]] bool operator!=(const ComboIterator<T> &o) const {
        return n != o.n;
    }

    reference operator*() const {
        return curr;
    }

    pointer operator->() const {
        return &curr;
    }

    self_type operator++(int) {
        std::next_permutation(bits.begin(), bits.end());
        set_curr();
        ++n;
        return *this;
    }

    self_type operator++() {
        self_type r = *this;
        std::next_permutation(bits.begin(), bits.end());
        set_curr();
        ++n;
        return r;
    }
};

size_t choose(size_t n, size_t k) {
    if (k == 0) return 1;
    return n * choose(n - 1, k - 1) / k;
}

template<class T>
struct Combos {
    const std::vector<T> &vals;
    size_t k;

    Combos(const std::vector<T> &vals, size_t k) : vals(vals), k(k) {
    }

    [[nodiscard]] ComboIterator<T> begin() const {
        return ComboIterator(vals, k, 0);
    }

    [[nodiscard]] ComboIterator<T> end() const {
        int j = choose(vals.size(), k);
        return ComboIterator(vals, k, j);
    }
};
