#include "groups.h"

//#include <utility>
#include <iterator>
#include <sstream>

namespace tc {
    Group::Group(int ngens, const std::vector<Mult> &rels, std::string name) : ngens(ngens), name(std::move(name)) {
        _mults.resize(ngens);
        for (int i = 0; i < ngens; i++) {
            _mults[i].resize(ngens, 2);
        }

        for (Mult m : rels) {
            if (m.gens[0] < m.gens[1])
                _mults[m.gens[0]][m.gens[1]] = m.mult;
            else
                _mults[m.gens[1]][m.gens[0]] = m.mult;
        }
    }

    void Group::setmult(Mult m) {
        if (m.gens[0] < m.gens[1])
            _mults[m.gens[0]][m.gens[1]] = m.mult;
        else
            _mults[m.gens[1]][m.gens[0]] = m.mult;
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
            g.setmult({off + m.gens[0], off + m.gens[1], m.mult});
        }

        std::stringstream ss;
        ss << name << "*" << other.name;
        g.name = ss.str();

        return g;
    }

    Group Group::power(int p) const {
        Group g(ngens * p);

        for (Mult m : get_mults()) {
            for (int off = 0; off < g.ngens; off += ngens) {
                g.setmult({off + m.gens[0], off + m.gens[1], m.mult});
            }
        }

        std::stringstream ss;
        ss << name << "^" << p;
        g.name = ss.str();

        return g;
    }

    Group operator*(const Group &g, const Group &h) {
        return g.product(h);
    }

    Group operator^(const Group &g, const int &p) {
        return g.power(p);
    }

    Group schlafli(const std::vector<int> &mults, const std::string name) {
        int ngens = mults.size() + 1;

        Group g(ngens, {}, name);

        for (int i = 0; i < mults.size(); i++) {
            g.setmult({i, i + 1, mults[i]});
        }

        return g;
    }

    Group schlafli(const std::vector<int> &mults) {
        std::stringstream ss;
        ss << "[";
        if (!mults.empty()) {
            copy(mults.begin(), mults.end() - 1, std::ostream_iterator<int>(ss, ","));
            ss << mults.back();
        }
        ss << "]";

        return schlafli(mults, ss.str());
    }

    namespace group {
        Group A(const int dim) {
            if (dim == 0)
                return Group(0, {}, "A(0)");

            const std::vector<int> &mults = std::vector<int>(dim - 1, 3);

            std::stringstream ss;
            ss << "A(" << dim << ")";

            return schlafli(mults, ss.str());
        }

        Group B(const int dim) {
            std::vector<int> mults(dim - 1, 3);
            mults[0] = 4;

            std::stringstream ss;
            ss << "B(" << dim << ")";

            return schlafli(mults, ss.str());
        }

        Group D(const int dim) {
            std::vector<int> mults(dim - 1, 3);
            mults[dim - 2] = 2;
            Group g = schlafli(mults);
            g.setmult({1, dim - 1, 3});

            std::stringstream ss;
            ss << "D(" << dim << ")";
            g.name = ss.str();

            return g;
        }

        Group E(const int dim) {
            std::vector<int> mults(dim - 1, 3);
            mults[dim - 2] = 2;
            Group g = schlafli(mults);
            g.setmult({2, dim - 1, 3});

            std::stringstream ss;
            ss << "E(" << dim << ")";
            g.name = ss.str();

            return g;
        }

        Group F4() {
            return schlafli({3, 4, 3}, "F4");
        }

        Group G2() {
            return schlafli({6}, "G2");
        }

        Group H(const int dim) {
            std::vector<int> mults(dim - 1, 3);
            mults[0] = 5;

            std::stringstream ss;
            ss << "H(" << dim << ")";

            return schlafli(mults, ss.str());
        }

        Group I2(const int n) {
            std::stringstream ss;
            ss << "I2(" << n << ")";

            return schlafli({n}, ss.str());
        }

        Group T(const int n, const int m) {
            Group g = I2(n) * I2(m);

            std::stringstream ss;
            ss << "T(" << n << "," << m << ")";
            g.name = ss.str();

            return g;
        }

        Group T(const int n) {
            Group g = I2(n) ^2;

            std::stringstream ss;
            ss << "T(" << n << ")";
            g.name = ss.str();

            return g;
        }
    }
}