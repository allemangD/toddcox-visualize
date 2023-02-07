#include <tc/core.hpp>

#include <cassert>
#include <numeric>

namespace tc {
    Group::Group(size_t rank) : _rank(rank), _mults(_rank * _rank, 2), _gens(_rank) {
        std::iota(_gens.begin(), _gens.end(), 0);

        for (int idx = 0; idx < rank; ++idx) {
            set(idx, idx, 1);
        }
    }

    void Group::set(size_t u, size_t v, Mult m) {
        assert(u < rank());
        assert(v < rank());

        _mults[u * rank() + v] = m;
        _mults[v * rank() + u] = m;
    }

    [[nodiscard]] Mult Group::get(size_t u, size_t v) const {
        assert(u < rank());
        assert(v < rank());

        return _mults[u * rank() + v];
    }

    [[nodiscard]] size_t Group::rank() const {
        return _rank;
    }

    [[nodiscard]] std::vector<size_t> Group::gens() const {
        return _gens;
    }

    [[nodiscard]] Group Group::sub(std::vector<size_t> const &gens) const {
        Group res(gens.size());
        res._gens = gens;

        for (int i = 0; i < gens.size(); ++i) {
            for (int j = i; j < gens.size(); ++j) {
                res.set(i, j, get(gens[i], gens[j]));
            }
        }

        return res;
    }

    [[nodiscard]] std::vector<Group> Group::subs(size_t rank) const {
        std::vector<bool> mask(_rank, false);
        std::fill(mask.begin(), mask.begin() + rank, true);

        std::vector<size_t> sub_gens(rank);

        std::vector<Group> res;

        do {
            for (int i = 0, j = 0; i < _rank; ++i) {
                if (mask[i]) sub_gens[j++] = i;
            }
            res.push_back(sub(sub_gens));
        } while (std::next_permutation(
            mask.begin(),
            mask.end(),
            std::greater<>()
        ));

        return res;
    }
}
