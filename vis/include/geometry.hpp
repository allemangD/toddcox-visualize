#pragma once

#include <tc/core.hpp>
#include <cmath>
#include <optional>

size_t get_key_from_gens(std::vector<int> &gens) {
    size_t key = 0;
    for (const auto gen : gens) {
        key += (1u << (unsigned)gen);
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

struct SimplexesList;

struct Simplexes {
    int dim;
    std::vector<int> vals;

    explicit Simplexes(int dim): dim(dim) {}
    Simplexes(int dim, std::vector<int> &vals): dim(dim), vals(vals) {}
    Simplexes(SimplexesList sl);

    size_t size() {
        return vals.size();
    }
};

struct SimplexesList {
    int dim;
    std::vector<int> vals;
    int elem_size;
    Simplexes temp;

    explicit SimplexesList(Simplexes s) : dim(s.dim), elem_size(s.size()), temp(s.dim) {
        temp.vals.reserve(s.size());
    }

    void reserve(size_t i) {
        vals.reserve(elem_size * i);
    }

    Simplexes& get(size_t i) {
        temp.vals.clear();
        temp.vals.insert(temp.vals.end(), vals.begin()+elem_size*i, vals.begin()+elem_size*(i+1));
        return temp;
    }

    void push_back(Simplexes s) {
        vals.insert(vals.end(), s.vals.begin(), s.vals.end());
    }
};

Simplexes::Simplexes(SimplexesList sl): Simplexes(sl.dim, sl.vals) {}

struct GeomGen {
    std::vector<std::vector<std::optional<tc::Cosets>>> coset_memo;
    tc::Group &context;

    explicit GeomGen(tc::Group &g): context(g) {
        size_t num_sg = std::pow(2, g.ngens);
        coset_memo.resize(num_sg);
        for (size_t i = 0; i < num_sg; i++) {
            auto num_sg_sg = std::pow(2, num_gens_from_key(i));
            coset_memo[i].resize(num_sg_sg, std::nullopt);
        }
    }

    std::vector<int> group_gens() {
        std::vector<int> gens(context.ngens);
        for (int i=0; i < context.ngens; i++) {
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

    Simplexes recontext(std::vector<int> &g_gens, std::vector<int> &sg_gens, Simplexes &items) {
        auto s_sg_gens = prepare_gens(g_gens, sg_gens);
        auto table = solve_g(g_gens);
        auto path = solve_g(sg_gens).path;

        auto coset_map = [table](int coset, int gen){return table.get(coset,gen);};

        auto map = path.walk<int,int>(0, s_sg_gens, coset_map);

        Simplexes ret(items.dim);
        ret.vals.reserve(items.size());
        for (const auto val : items.vals) {
            ret.vals.push_back(map[val]);
        }
        return ret;
    }

    Simplexes tile(std::vector<int> &g_gens, std::vector<int> &sg_gens, Simplexes &items) {
        Simplexes base = recontext(g_gens, sg_gens, items);
        auto s_sg_gens = prepare_gens(g_gens, sg_gens);
        auto table = solve_g(g_gens);
        auto path = _solve(g_gens, sg_gens).path;

        auto simplex_map = [table](const Simplexes& items, int gen) -> Simplexes {
            Simplexes ret(items.dim);
            ret.vals.reserve(items.vals.size());
            // Move the simplexes
            for (const auto coset : items.vals) {
                ret.vals.push_back(table.get(coset,gen));
            }
            // Reorient the simplexes
            if (ret.dim > 0) {
                for (int i = 0; i < ret.size(); i += ret.dim+1) {
                    std::swap(ret.vals[i],ret.vals[i+1]);
                }
            }
            return ret;
        };

        SimplexesList ret(base);
        path.walk<SimplexesList, Simplexes, int>(ret, base, group_gens(), simplex_map);

        return ret;
    }
};