#pragma once

#include <tc/core.hpp>
#include <cmath>
#include <optional>
#include <numeric>
#include <iostream>

#include <geometry.hpp>

#include "combo_iterator.hpp"

/**
 * Produce a list of all generators for the group context. The range [0..group.ngens).
 */
std::vector<int> generators(const tc::Group &context) {
    std::vector<int> g_gens(context.ngens);
    std::iota(g_gens.begin(), g_gens.end(), 0);
    return g_gens;
}

/**
 * Determine which of g_gens are the correct names for sg_gens within the current context
 */
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
}

/**
 * Solve the cosets generated by sg_gens within the subgroup generated by g_gens of the group context
 */
tc::Cosets solve(
    const tc::Group &context,
    const std::vector<int> &g_gens,
    const std::vector<int> &sg_gens
) {
    const auto proper_sg_gens = recontext_gens(context, g_gens, sg_gens);
    return context.subgroup(g_gens).solve(proper_sg_gens);
}

/**
 * Apply some context transformation to all primitives of this mesh.
 */
template<unsigned N>
std::vector<Primitive<N>> apply(std::vector<Primitive<N>> prims, const tc::Cosets &table, int gen) {
    for (auto &prim : prims) {
        prim.apply(table, gen);
    }
    return prims;
}

/**
 * Convert the indexes of this mesh to those of a different context, using g_gens to build the parent context and sg_gens to build this context.
 */
template<unsigned N>
[[nodiscard]]
std::vector<Primitive<N>> recontext(
    std::vector<Primitive<N>> prims,
    const tc::Group &context,
    const std::vector<int> &g_gens,
    const std::vector<int> &sg_gens
) {
    const auto proper_sg_gens = recontext_gens(context, g_gens, sg_gens);
    const auto table = solve(context, g_gens, {});
    const auto path = solve(context, sg_gens, {}).path;

    auto map = path.template walk<int, int>(0, proper_sg_gens, [table](int coset, int gen) {
        return table.get(coset, gen);
    });

    std::vector<Primitive<N>> res(prims);
    for (Primitive<N> &prim : res) {
        for (auto &ind : prim.inds) {
            ind = map[ind];
        }
    }

    return res;
}

/**
 * Union several meshes of the same dimension
 */
template<unsigned N>
std::vector<Primitive<N>> merge(const std::vector<std::vector<Primitive<N>>> &meshes) {
    size_t size = 0;
    for (const auto &mesh : meshes) {
        size += mesh.size();
    }

    std::vector<Primitive<N>> res;
    res.reserve(size);
    for (const auto &mesh : meshes) {
        res.insert(res.end(), mesh.begin(), mesh.end());
    }

    return res;
}

template<unsigned N>
[[nodiscard]]
std::vector<std::vector<Primitive<N>>> each_tile(
    std::vector<Primitive<N>> prims,
    const tc::Group &context,
    const std::vector<int> &g_gens,
    const std::vector<int> &sg_gens
) {
    std::vector<Primitive<N>> base = recontext(prims, context, g_gens, sg_gens);
    const auto proper_sg_gens = recontext_gens(context, g_gens, sg_gens);

    const auto table = solve(context, g_gens, {});
    const auto path = solve(context, g_gens, sg_gens).path;

    auto _gens = generators(context);

    auto res = path.walk<std::vector<Primitive<N>>, int>(base, generators(context), [&](auto from, auto gen) {
        return apply(from, table, gen);
    });

    return res;
}

template<unsigned N>
[[nodiscard]]
std::vector<Primitive<N>> tile(
    std::vector<Primitive<N>> prims,
    const tc::Group &context,
    const std::vector<int> &g_gens,
    const std::vector<int> &sg_gens
) {
    auto res = each_tile<N>(prims, context, g_gens, sg_gens);

    return merge(res);
}

/**
 * Produce a mesh of higher dimension by fanning a single point to all primitives in this mesh.
 */
template<unsigned N>
[[nodiscard]]
std::vector<Primitive<N + 1>> fan(std::vector<Primitive<N>> prims, int root) {
    std::vector<Primitive<N + 1>> res(prims.size());
    std::transform(prims.begin(), prims.end(), res.begin(),
        [root](const Primitive<N> &prim) {
            return Primitive<N + 1>(prim, root);
        }
    );
    return res;
}

/**
 * Produce a mesh of primitives that fill out the volume of the subgroup generated by generators g_gens within the group context
 */
template<unsigned N>
std::vector<Primitive<N>> triangulate(
    const tc::Group &context,
    const std::vector<int> &g_gens
) {
    if (g_gens.size() + 1 != N) // todo make static assert
        throw std::logic_error("g_gens size must be one less than N");

    const auto &combos = Combos(g_gens, g_gens.size() - 1);

    std::vector<std::vector<Primitive<N>>> meshes;

    for (const auto &sg_gens : combos) {
        auto base = triangulate<N - 1>(context, sg_gens);
        auto raised = tile(base, context, g_gens, sg_gens);
        raised.erase(raised.begin(), raised.begin() + base.size());
        meshes.push_back(fan(raised, 0));
    }

    return merge(meshes);
}

/**
 * Single-index primitives should not be further triangulated.
 */
template<>
std::vector<Primitive<1>> triangulate(
    const tc::Group &context,
    const std::vector<int> &g_gens
) {
    if (not g_gens.empty()) // todo make static assert
        throw std::logic_error("g_gens must be empty for a trivial Mesh");

    std::vector<Primitive<1>> res;
    res.emplace_back();
    return res;
}

template<unsigned N, class T>
auto hull(const tc::Group &group, T all_sg_gens, const std::vector<std::vector<int>> &exclude) {
    std::vector<std::vector<Primitive<N>>> parts;
    auto g_gens = generators(group);
    for (const std::vector<int> &sg_gens : all_sg_gens) {
        bool excluded = false;
        for (const auto &test : exclude) {
            if (sg_gens == test) {
                excluded = true;
                break;
            }
        }
        if (excluded) continue;

        const auto &base = triangulate<N>(group, sg_gens);
        const auto &tiles = each_tile(base, group, g_gens, sg_gens);
        for (const auto &tile : tiles) {
            parts.push_back(tile);
        }
    }
    return parts;
}