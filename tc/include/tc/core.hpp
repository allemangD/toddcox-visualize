#pragma once

#include <array>
#include <functional>
#include <vector>
#include <string>

namespace tc {
    struct Action {
        int from_idx = -1;
        int gen = -1;

        Action() = default;

        Action(const Action &) = default;

        Action(int from_idx, int gen);
    };

    struct Path {
        std::vector<Action> path;

        Path() = default;

        Path(const Path &) = default;

        void add_row();

        [[nodiscard]] Action get(int to_idx) const;

        void put(int from_idx, int gen, int to_idx);

        template<class C, class T, class E>
        void walk(
            C& res,
            T start,
            std::vector<E> gens,
            std::function<T(const T &, const E &)> op
        ) const {
            size_t s = size();
            res.reserve(s);
            res.push_back(start);

            for (int i = 1; i < s; ++i) {
                auto &action = path[i];
                auto &from = res.get(action.from_idx);
                auto &val = gens[action.gen];
                res.push_back(op(from,val));
            }
        }

        template<class T, class E>
        [[nodiscard]] std::vector<T> walk(
            T start,
            std::vector<E> gens,
            std::function<T(const T &, const E &)> op
        ) const {
            std::vector<T> res;
            res.reserve(size());
            res.push_back(start);

            for (int i = 1; i < size(); ++i) {
                auto &action = path[i];
                auto &from = res[action.from_idx];
                auto &val = gens[action.gen];
                res.push_back(op(from, val));
            }

            return res;
        }

        template<class T>
        [[nodiscard]] std::vector<T> walk(
            T start,
            std::function<T(const T &, const int &)> op
        ) const {
            std::vector<T> res;
            res.reserve(size());
            res.push_back(start);

            for (int i = 1; i < size(); ++i) {
                auto &action = path[i];
                auto &from = res[action.from_idx];
                auto &val = action.gen;
                res[i] = op(from, val);
            }

            return res;
        }

        [[nodiscard]] size_t size() const;
    };

    struct Cosets {
        int ngens;
        std::vector<int> data;
        Path path;

        Cosets(const Cosets &) = default;

        explicit Cosets(int ngens);

        void add_row();

        void put(int coset, int gen, int target);

        void put(int idx, int target);

        [[nodiscard]] int get(int coset, int gen) const;

        [[nodiscard]] int get(int idx) const;

        [[nodiscard]] size_t size() const;
    };

    struct Rel {
        std::array<int, 2> gens;
        int mult;

        Rel() = default;

        Rel(const Rel &) = default;

        Rel(int a, int b, int m);

        [[nodiscard]] Rel shift(int off) const;
    };

    struct SubGroup;

    struct Group {
        int ngens;
        std::vector<std::vector<int>> _mults;
        std::string name;

        Group(const Group &) = default;

        explicit Group(int ngens, const std::vector<Rel> &rels = {}, std::string name = "G");

        void set(const Rel &r);

        [[nodiscard]] int get(int a, int b) const;

        [[nodiscard]] std::vector<Rel> rels() const;

        [[nodiscard]] SubGroup subgroup(const std::vector<int> &gens) const;

        [[nodiscard]] Group product(const Group &other) const;

        [[nodiscard]] Group power(int p) const;

        [[nodiscard]] Cosets solve(const std::vector<int> &sub_gens = {}) const;
    };

    struct SubGroup : public Group {
        std::vector<int> gen_map;
        const Group &parent;

        SubGroup(const Group &parent, std::vector<int> gen_map);
    };

    Group operator*(const Group &g, const Group &h);

    Group operator^(const Group &g, int p);
}
