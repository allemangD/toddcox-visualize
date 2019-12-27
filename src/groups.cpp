#include "groups.h"

namespace tc {
    Group::Group(int ngens, const std::vector<Mult> &rels) : ngens(ngens) {
        _mults.resize(ngens);
        for (int i = 0; i < ngens; i++) {
            _mults[i].resize(ngens, 2);
        }

        for (Mult m : rels) {
            if (m.gen0 < m.gen1)
                _mults[m.gen0][m.gen1] = m.mult;
            else
                _mults[m.gen1][m.gen0] = m.mult;
        }
    }

    void Group::setmult(Mult m) {
        if (m.gen0 < m.gen1)
            _mults[m.gen0][m.gen1] = m.mult;
        else
            _mults[m.gen1][m.gen0] = m.mult;
    }

    std::vector<Mult> Group::get_mults() const {
        std::vector<Mult> mults;
        for (int i = 0; i < ngens - 1; i++) {
            for (int j = i + 1; j < ngens; j++) {
                mults.push_back({i, j, _mults[i][j]});
            }
        }
        return mults;
    }

    Group Group::product(const Group &other) const {
        int off = ngens;

        Group g(ngens + other.ngens, get_mults());

        for (Mult m : other.get_mults()) {
            g.setmult({off + m.gen0, off + m.gen1, m.mult});
        }

        return g;
    }

    Group Group::power(int p) const {
        Group g(ngens * p);

        for (Mult m : get_mults()) {
            for (int off = 0; off < g.ngens; off += ngens) {
                g.setmult({off + m.gen0, off + m.gen1, m.mult});
            }
        }

        return g;
    }

    Group operator*(const Group &g, const Group &h) {
        return g.product(h);
    }

    Group operator^(const Group &g, const int &p) {
        return g.power(p);
    }

    Group schlafli(const std::vector<int> &mults) {
        int ngens = mults.size() + 1;
        Group g(ngens);
        for (int i = 0; i < mults.size(); i++) {
            g.setmult({i, i + 1, mults[i]});
        }
        return g;
    }

    namespace group {
        Group A(const int dim) {
            if (dim == 0)
                return Group(0);

            return schlafli(std::vector<int>(dim - 1, 3));
        }

        Group B(const int dim) {
            std::vector<int> mults(dim - 1, 3);
            mults[0] = 4;
            return schlafli(mults);
        }

        Group D(const int dim) {
            std::vector<int> mults(dim - 1, 3);
            mults[dim - 2] = 2;
            Group g = schlafli(mults);
            g.setmult({1, dim - 1, 3});
            return g;
        }

        Group E(const int dim) {
            std::vector<int> mults(dim - 1, 3);
            mults[dim - 2] = 2;
            Group g = schlafli(mults);
            g.setmult({2, dim - 1, 3});
            return g;
        }

        Group F4() {
            return schlafli({3, 4, 3});
        }

        Group G2() {
            return schlafli({6});
        }

        Group H(const int dim) {
            std::vector<int> mults(dim - 1, 3);
            mults[0] = 5;
            return schlafli(mults);
        }

        Group I2(const int n) {
            return schlafli({n});
        }

        Group T(const int n, const int m) {
            return I2(n) * I2(m);
        }

        Group T(const int n) {
            return I2(n) ^ 2;
        }
    }
}