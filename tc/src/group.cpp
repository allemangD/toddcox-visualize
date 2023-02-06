#include <tc/core.hpp>

#include <cassert>

namespace tc {
    Group::Group(size_t rank) : _rank(rank), _mults(_rank * _rank, 2) {
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

    [[nodiscard]] Group Group::sub(std::vector<size_t> const &idxs) const {
        Group res(idxs.size());

        for (int i = 0; i < idxs.size(); ++i) {
            for (int j = i; j < idxs.size(); ++j) {
                res.set(i, j, get(idxs[i], idxs[j]));
            }
        }

        return res;
    }
}
