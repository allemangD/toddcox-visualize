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

template<unsigned N>
struct Primitive {
    std::array<unsigned, N> inds;

    Primitive() = default;

    Primitive(const Primitive<N> &) = default;

    Primitive(const Primitive<N - 1> &sub, unsigned root) {
        std::copy(sub.inds.begin(), sub.inds.end(), inds.begin());
        inds[N - 1] = root;
    }

    ~Primitive() = default;

    inline void flip() {
        if (N > 1) std::swap(inds[0], inds[1]);
    }

    void apply(const tc::Cosets &table, int gen) {
        for (auto &ind : inds) {
            ind = table.get(ind, gen);
        }
        flip();
    }
};

std::vector<int> gens(const tc::Group &context) {
    std::vector<int> g_gens(context.ngens);
    std::iota(g_gens.begin(), g_gens.end(), 0);
    return g_gens;
}

std::vector<int> recontext_gens(
    const tc::Group &context,
    std::vector<int> g_gens,
    std::vector<int> sg_gens) {

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

//    std::sort(g_gens.begin(), g_gens.end());
//
//    std::vector<int> inv_g_map(g_gens.size());
//    for (int i = 0; i < g_gens.size(); ++i) {
//        inv_g_map[g_gens[i]] = i;
//    }
//
//    std::transform(sg_gens.begin(), sg_gens.end(), sg_gens.begin(),
//        [inv_g_map](const auto &gen) {
//            return inv_g_map[gen];
//        }
//    );
//
//    std::sort(sg_gens.begin(), sg_gens.end());
//    return sg_gens;
}

int get_parity(
    const tc::Group &context,
    const std::vector<int> &g_gens,
    const std::vector<int> &sg_gens
) {
    if (g_gens.size() != sg_gens.size() + 1) return 0;

    const auto proper_sg_gens = recontext_gens(context, g_gens, sg_gens);

    int i = 0;
    for (; i < sg_gens.size(); ++i) {
        if (proper_sg_gens[i] != i) {
            break;
        }
    }

    return i & 1;
}

tc::Cosets solve(
    const tc::Group &context,
    const std::vector<int> &g_gens,
    const std::vector<int> &sg_gens
) {
    const auto proper_sg_gens = recontext_gens(context, g_gens, sg_gens);
    return context.subgroup(g_gens).solve(proper_sg_gens);
}

tc::Cosets solve_sg(
    const tc::Group &context,
    const std::vector<int> &sg_gens
) {
    return solve(context, gens(context), sg_gens);
}

tc::Cosets solve_g(
    const tc::Group &context,
    const std::vector<int> &g_gens
) {
    std::vector<int> sg_gens;
    return solve(context, g_gens, sg_gens);
}

tc::Cosets solve(
    const tc::Group &context
) {
    std::vector<int> sg_gens;
    return solve_sg(context, sg_gens);
}

template<unsigned N>
struct Mesh {
    std::vector<Primitive<N>> prims;

    Mesh() : prims() {}

    Mesh(const Mesh<N> &) = default;

    explicit Mesh(std::vector<Primitive<N>> &prims) : prims(prims) {}

    [[nodiscard]] size_t size() const {
        return prims.size();
    }

    void apply(const tc::Cosets &table, int gen) {
        for (auto &prim : prims) {
            prim.apply(table, gen);
        }
    }

    void flip() {
        for (auto &prim : prims) {
            prim.flip();
        }
    }

    [[nodiscard]]
    Mesh<N> recontext(
        const tc::Group &context,
        const std::vector<int> &g_gens,
        const std::vector<int> &sg_gens
    ) const {
        const auto proper_sg_gens = recontext_gens(context, g_gens, sg_gens);
        const auto table = solve_g(context, g_gens);
        const auto path = solve_g(context, sg_gens).path;

        auto map = path.template walk<int, int>(0, proper_sg_gens, [table](int coset, int gen) {
            return table.get(coset, gen);
        });

        Mesh<N> res = *this;
        for (Primitive<N> &prim : res.prims) {
            for (auto &ind : prim.inds) {
                ind = map[ind];
            }
        }

        if (get_parity(context, g_gens, sg_gens) == 1)
            res.flip();

        return res;
    }

    [[nodiscard]]
    Mesh<N> tile(
        const tc::Group &context,
        const std::vector<int> &g_gens,
        const std::vector<int> &sg_gens
    ) const {
        Mesh<N> base = recontext(context, g_gens, sg_gens);
        const auto proper_sg_gens = recontext_gens(context, g_gens, sg_gens);

        const auto table = solve_g(context, g_gens);
        const auto path = solve(context, g_gens, sg_gens).path;

        const auto all = path.template walk<Mesh<N>, int>(base, gens(context), [table](Mesh<N> from, int gen) {
            from.apply(table, gen);
            return from;
        });

        return merge(all);
    }

    [[nodiscard]]
    Mesh<N + 1> fan(int root) const {
        std::vector<Primitive<N + 1>> res(prims.size());
        std::transform(prims.begin(), prims.end(), res.begin(),
            [root](const Primitive<N> &prim) {
                return Primitive<N + 1>(prim, root);
            }
        );
        return Mesh<N + 1>(res);
    }
};

template<unsigned N>
Mesh<N> merge(const std::vector<Mesh<N>> &meshes) {
    size_t size = 0;
    for (const auto &mesh : meshes) {
        size += mesh.size();
    }

    std::vector<Primitive<N>> prims;
    prims.reserve(size);
    for (const auto &mesh : meshes) {
        prims.insert(prims.end(), mesh.prims.begin(), mesh.prims.end());
    }

    return Mesh(prims);
}

template<unsigned N>
Mesh<N> triangulate(
    const tc::Group &context,
    const std::vector<int> &g_gens
) {
    if (g_gens.size() + 1 != N)
        throw std::logic_error("g_gens size must be one less than N");

    const auto &combos = Combos(g_gens, g_gens.size() - 1);

    std::vector<Mesh<N>> meshes;
    for (const auto &sg_gens : combos) {
        Mesh<N - 1> base = triangulate<N - 1>(context, sg_gens);
        Mesh<N - 1> raised = base.tile(context, g_gens, sg_gens);
        raised.prims.erase(raised.prims.begin(), raised.prims.begin() + base.size());
        Mesh<N> fan = raised.fan(0);
        meshes.push_back(fan);
    }

    return merge(meshes);
}

template<>
Mesh<1> triangulate<1>(
    const tc::Group &context,
    const std::vector<int> &g_gens
) {
    if (not g_gens.empty())
        throw std::logic_error("g_gens must be empty for a trivial Mesh");

    Mesh<1> res;
    res.prims.emplace_back();
    return res;
}
