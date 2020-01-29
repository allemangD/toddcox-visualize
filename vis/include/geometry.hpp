#pragma once

#include <tc/core.hpp>
#include <cmath>
#include <optional>
#include <numeric>
#include <iostream>
#include "combo_iterator.hpp"

size_t get_key_from_gens(std::vector<int> &gens) {
    size_t key = 0;
    for (const auto gen : gens) {
        key += (1u << (unsigned) gen);
    }
    return key;
}

size_t num_gens_from_key(size_t key) {
    size_t mask = 1;
    size_t count = 0;
    while (mask <= key) {
        if (key & mask)
            count++;
        mask <<= 1u;
    }
    return count;
}

struct Simplexes {
    int dim;
    std::vector<int> vals;

    Simplexes() : dim(0), vals() {}

    explicit Simplexes(int dim) : dim(dim), vals() {}

    Simplexes(int dim, std::vector<int> &vals) : dim(dim), vals(vals) {}

    explicit Simplexes(const std::vector<Simplexes> &parts) {
        dim = parts[0].dim;

        size_t count = 0;
        for (const auto &part : parts) {
            count += part.size();
        }
        vals.reserve(count);

        for (const auto &part : parts) {
            vals.insert(vals.end(), part.vals.begin(), part.vals.end());
        }
    }

    [[nodiscard]] size_t size() const {
        return vals.size();
    }

    void flip() {
        if (dim == 0)
            return;
        for (int i = 0; i < vals.size(); i += dim + 1) {
            std::swap(vals[i], vals[i + 1]);
        }
    }
};

struct GeomGen {
    std::vector<std::vector<std::optional<tc::Cosets>>> coset_memo;
    std::vector<std::optional<Simplexes>> triangulate_memo;
    tc::Group &context;

    explicit GeomGen(tc::Group &g) : context(g) {
        size_t num_sg = std::pow(2, g.ngens);
        coset_memo.resize(num_sg);
        triangulate_memo.resize(num_sg);
        for (size_t i = 0; i < num_sg; i++) {
            auto num_sg_sg = std::pow(2, num_gens_from_key(i));
            coset_memo[i].resize(num_sg_sg, std::nullopt);
        }
    }

    std::vector<int> group_gens() {
        std::vector<int> gens(context.ngens);
        for (int i = 0; i < context.ngens; i++) {
            gens[i] = i;
        }
        return gens;
    }

    std::vector<int> prepare_gens(std::vector<int> &g_gens, std::vector<int> &sg_gens) {
        std::sort(g_gens.begin(), g_gens.end());

        int inv_gen_map[context.ngens];
        for (size_t i = 0; i < g_gens.size(); i++) {
            inv_gen_map[g_gens[i]] = i;
        }

        std::vector<int> s_sg_gens;
        s_sg_gens.reserve(sg_gens.size());
        for (const auto gen : sg_gens) {
            s_sg_gens.push_back(inv_gen_map[gen]);
        }
        std::sort(s_sg_gens.begin(), s_sg_gens.end());

        return s_sg_gens;
    }

    int get_parity(std::vector<int> &g_gens, std::vector<int> &sg_gens) {
        if (g_gens.size() != sg_gens.size() + 1)
            return 0;
        auto s_sg_gens = prepare_gens(g_gens, sg_gens);
        const int loop_max = g_gens.size() - 1;
        for (int i = 0; i < loop_max; i++) {
            if (s_sg_gens[i] != i)
                return i % 2;
        }
        return loop_max % 2;
    }

    tc::Cosets _solve(std::vector<int> &g_gens, std::vector<int> &sg_gens) {
        auto s_sg_gens = prepare_gens(g_gens, sg_gens);

        size_t group_key = get_key_from_gens(g_gens);
        size_t subgroup_key = get_key_from_gens(s_sg_gens);

        if (!coset_memo[group_key][subgroup_key]) {
            tc::SubGroup g = context.subgroup(g_gens);
            coset_memo[group_key][subgroup_key] = g.solve(s_sg_gens);
        }
        return *coset_memo[group_key][subgroup_key];
    }

    tc::Cosets solve(std::vector<int> g_gens, std::vector<int> sg_gens) {
        return _solve(g_gens, sg_gens);
    }

    tc::Cosets solve_sg(std::vector<int> &sg_gens) {
        auto g_gens = group_gens();
        return _solve(g_gens, sg_gens);
    }

    tc::Cosets solve_g(std::vector<int> &g_gens) {
        std::vector<int> sg_gens;
        return _solve(g_gens, sg_gens);
    }

    tc::Cosets solve() {
        std::vector<int> sg_gens;
        return solve_sg(sg_gens);
    }

    Simplexes recontext(std::vector<int> &g_gens, std::vector<int> &sg_gens, const Simplexes &items) {
        auto s_sg_gens = prepare_gens(g_gens, sg_gens);
        auto table = solve_g(g_gens);
        auto path = solve_g(sg_gens).path;

        auto coset_map = [table](int coset, int gen) { return table.get(coset, gen); };

        auto map = path.walk<int, int>(0, s_sg_gens, coset_map);

        Simplexes ret(items.dim);
        ret.vals.reserve(items.size());
        for (const auto val : items.vals) {
            ret.vals.push_back(map[val]);
        }
        if (get_parity(g_gens, sg_gens) == 1)
            ret.flip();
        return ret;
    }

    Simplexes tile(std::vector<int> &g_gens, std::vector<int> &sg_gens, const Simplexes &items) {
        Simplexes base = recontext(g_gens, sg_gens, items);
        auto s_sg_gens = prepare_gens(g_gens, sg_gens);
        auto table = solve_g(g_gens);
        auto path = _solve(g_gens, sg_gens).path;

        auto simplex_map = [table](Simplexes from, int gen) -> Simplexes {
            for (auto &coset : from.vals) {
                coset = table.get(coset, gen);
            }
            from.flip();
            return from;
        };

        auto r = path.walk<Simplexes, int>(base, group_gens(), simplex_map);

        return Simplexes(r);
    }

    Simplexes triangulate(std::vector<int> &g_gens);

    Simplexes _triangulate(std::vector<int> &g_gens) {
        Simplexes S(g_gens.size());
        if (g_gens.empty()) {
            S.vals.push_back(0);
            return S;
        }
        for (std::vector<int> sg_gens : Combos(g_gens, g_gens.size() - 1)) {
            auto sub_simps = triangulate(sg_gens);
            int start = sub_simps.size();
            sub_simps = tile(g_gens, sg_gens, sub_simps);
            for (int l = start; l < sub_simps.size(); l += S.dim) {
                for (int m = l; m < l + S.dim; m++) {
                    S.vals.push_back(sub_simps.vals[m]);
                }
                S.vals.push_back(0);
            }
        }
        return S;
    }

};

Simplexes GeomGen::triangulate(std::vector<int> &g_gens) {
    int key = get_key_from_gens(g_gens);
    if (!triangulate_memo[key]) {
        triangulate_memo[key] = _triangulate(g_gens);
    }
    return *triangulate_memo[key];
}