#include <tc/core.hpp>

namespace tc {
    Cosets::Cosets(size_t rank)
        : _rank(rank), _order(0), _complete(false), _data() {}

    void Cosets::set(size_t coset, size_t gen, size_t target) {
        set(coset * rank() + gen, target);
    }

    [[nodiscard]] size_t Cosets::get(size_t coset, size_t gen) const {
        return get(coset * rank() + gen);
    }

    [[nodiscard]] bool Cosets::isset(size_t coset, size_t gen) const {
        return isset(coset * rank() + gen);
    }

    [[nodiscard]] size_t Cosets::rank() const {
        return _rank;
    }

    [[nodiscard]] size_t Cosets::order() const {
        return _order;
    }

    [[nodiscard]] bool Cosets::complete() const {
        return _complete;
    }

    [[nodiscard]] size_t Cosets::size() const {
        return _data.size();
    }

    void Cosets::add_row() {
        _data.resize(_data.size() + rank(), UNSET);
        _order++;
    }

    void Cosets::set(size_t idx, size_t target) {
        size_t coset = idx / rank();
        size_t gen = idx % rank();
        _data[idx] = target;
        _data[target * rank() + gen] = coset;
    }

    [[nodiscard]] size_t Cosets::get(size_t idx) const {
        return _data[idx];
    }

    [[nodiscard]] bool Cosets::isset(size_t idx) const {
        return get(idx) != UNSET;
    }

}