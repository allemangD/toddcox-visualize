#pragma once

#include <cstdlib>
#include <vector>
#include <algorithm>
#include <functional>

namespace tc {
    using Gen = uint8_t;
    using Mult = uint16_t;

    constexpr Mult FREE = Mult(-1);

    using Coset = uint32_t;
    constexpr Coset UNSET = Coset(-1);
    constexpr Coset UNBOUNDED = (Coset) (-1);

    using Rel = std::tuple<Gen, Gen, Mult>;

    struct Action {
        int from_idx = -1;
        int gen = -1;

        Action() = default;

        Action(const Action &) = default;

        Action(int from_idx, int gen)
            : from_idx(from_idx), gen(gen) {
        }
    };

    struct Path {
        std::vector<Action> path;

        Path() = default;

        Path(const Path &) = default;

        void add_row() {
            path.resize(path.size() + 1);
        }

        [[nodiscard]] Action get(int to_idx) const {
            return path[to_idx];
        }

        void put(int from_idx, int gen, int to_idx) {
            path[to_idx] = Action(from_idx, gen);
        }

        template<class C, class T, class E>
        void walk(
            C &res,
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
                res.push_back(op(from, val));
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

        [[nodiscard]] size_t size() const {
            return path.size();
        }
    };
}
