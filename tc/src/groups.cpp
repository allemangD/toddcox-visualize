#include "tc/groups.h"

#include <iterator>
#include <sstream>

namespace tc {
    Group::Group(int ngens, const std::vector<Rel> &rels, std::string name) : ngens(ngens), name(std::move(name)) {
        _mults.resize(ngens);
        for (int i = 0; i < ngens; i++) {
            _mults[i].resize(ngens, 2);
        }

        for (Rel rel : rels) {
            if (rel.gens[0] < rel.gens[1])
                _mults[rel.gens[0]][rel.gens[1]] = rel.mult;
            else
                _mults[rel.gens[1]][rel.gens[0]] = rel.mult;
        }
    }

    void Group::setmult(Rel rel) {
        if (rel.gens[0] < rel.gens[1])
            _mults[rel.gens[0]][rel.gens[1]] = rel.mult;
        else
            _mults[rel.gens[1]][rel.gens[0]] = rel.mult;
    }

    std::vector<Rel> Group::get_rels() const {
        std::vector<Rel> rels;
        for (int i = 0; i < ngens - 1; i++) {
            for (int j = i + 1; j < ngens; j++) {
                rels.push_back({i, j, _mults[i][j]});
            }
        }
        return rels;
    }

    Group Group::product(const Group &other) const {
        int off = ngens;

        Group g(ngens + other.ngens, get_rels());

        for (Rel rel : other.get_rels()) {
            g.setmult({off + rel.gens[0], off + rel.gens[1], rel.mult});
        }

        std::stringstream ss;
        ss << name << "*" << other.name;
        g.name = ss.str();

        return g;
    }

    Group Group::power(int p) const {
        Group g(ngens * p);

        for (Rel rel : get_rels()) {
            for (int off = 0; off < g.ngens; off += ngens) {
                g.setmult({off + rel.gens[0], off + rel.gens[1], rel.mult});
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

    Group schlafli(const std::vector<int> &mults, const std::string& name) {
        int ngens = (int) mults.size() + 1;

        Group g(ngens, {}, name);

        for (int i = 0; i < (int) mults.size(); i++) {
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