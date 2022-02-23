#include "tc/core.hpp"

#include <utility>
#include <sstream>
#include <algorithm>

namespace tc {
    Action::Action(int from_idx, int gen)
        : from_idx(from_idx), gen(gen) {
    }

    void Path::add_row() {
        path.resize(path.size() + 1);
    }

    Action Path::get(int to_idx) const {
        return path[to_idx];
    }

    void Path::put(int from_idx, int gen, int to_idx) {
        path[to_idx] = Action(from_idx, gen);
    }

    size_t Path::size() const {
        return path.size();
    }

    Cosets::Cosets(int ngens)
        : ngens(ngens) {
    }

    void Cosets::add_row() {
        data.resize(data.size() + ngens, -1);
        path.add_row();
    }

    void Cosets::put(int coset, int gen, int target) {
        data[coset * ngens + gen] = target;
        data[target * ngens + gen] = coset;

        if (path.get(target).from_idx == -1) {
            path.put(coset, gen, target);
        }
    }

    void Cosets::put(int idx, int target) {
        int coset = idx / ngens;
        int gen = idx % ngens;
        data[idx] = target;
        data[target * ngens + gen] = coset;

        if (path.get(target).from_idx == -1) {
            path.put(coset, gen, target);
        }
    }

    int Cosets::get(int coset, int gen) const {
        return data[coset * ngens + gen];
    }

    int Cosets::get(int idx) const {
        return data[idx];
    }

    size_t Cosets::size() const {
        return path.size();
    }

    Rel::Rel(int a, int b, int m)
        : gens({a, b}), mult(m) {
    }

    Rel Rel::shift(int off) const {
        return Rel(gens[0] + off, gens[1] + off, mult);
    }

    Group::Group(int ngens, const std::vector<Rel> &rels, std::string name)
        : ngens(ngens), name(std::move(name)) {
        _mults.resize(ngens);

        for (auto &mult : _mults) {
            mult.resize(ngens, 2);
        }

        for (const auto &rel : rels) {
            set(rel);
        }
    }

    void Group::set(const Rel &r) {
        _mults[r.gens[0]][r.gens[1]] = r.mult;
        _mults[r.gens[1]][r.gens[0]] = r.mult;
    }

    int Group::get(int a, int b) const {
        return _mults[a][b];
    }

    std::vector<Rel> Group::rels() const {
        std::vector<Rel> res;
        for (int i = 0; i < ngens - 1; ++i) {
            for (int j = i + 1; j < ngens; ++j) {
                res.emplace_back(i, j, get(i, j));
            }
        }
        return res;
    }

    SubGroup Group::subgroup(const std::vector<int> &gens) const {
        return SubGroup(*this, gens);
    }

    Group Group::product(const Group &other) const {
        std::stringstream ss;
        ss << name << "*" << other.name;

        Group g(ngens + other.ngens, rels(), ss.str());

        for (const auto &rel : other.rels()) {
            g.set(rel.shift(ngens));
        }

        return g;
    }

    Group Group::power(int p) const {
        std::stringstream ss;
        ss << name << "^" << p;

        Group g(ngens * p, {}, ss.str());
        for (const auto &rel : rels()) {
            for (int off = 0; off < g.ngens; off += ngens) {
                g.set(rel.shift(off));
            }
        }

        return g;
    }

    SubGroup::SubGroup(const Group &parent, std::vector<int> gen_map)
        : Group(gen_map.size()), parent(parent) {

        std::sort(gen_map.begin(), gen_map.end());
        this->gen_map = gen_map;

        for (size_t i = 0; i < gen_map.size(); ++i) {
            for (size_t j = 0; j < gen_map.size(); ++j) {
                int mult = parent.get(gen_map[i], gen_map[j]);
                set(Rel(i, j, mult));
            }
        }
    }

    Group operator*(const Group &g, const Group &h) {
        return g.product(h);
    }

    Group operator^(const Group &g, int p) {
        return g.power(p);
    }
}
