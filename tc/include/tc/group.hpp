#pragma once

#include <sstream>

#include <tc/pair_map.hpp>

namespace tc {
    struct Group;
    struct SubGroup;

    struct Group {
        int ngens;
        tc::pair_map<int> _mults;

        Group(const Group &) = default;

        explicit Group(int ngens, const std::vector<Rel> &rels = {})
            : ngens(ngens), _mults(ngens, 2) {

            for (const auto &rel: rels) {
                set(rel);
            }
        }

        void set(const Rel &r) {
            _mults(r.gens[0], r.gens[1]) = r.mult;
        }

        [[nodiscard]] int get(int a, int b) const {
            return _mults(a, b);
        }

        [[nodiscard]] std::vector<Rel> rels() const {
            std::vector<Rel> res;
            for (int i = 0; i < ngens - 1; ++i) {
                for (int j = i + 1; j < ngens; ++j) {
                    res.emplace_back(i, j, get(i, j));
                }
            }
            return res;
        }

        [[nodiscard]] SubGroup subgroup(const std::vector<int> &gens) const;

        [[nodiscard]] Cosets solve(const std::vector<int> &sub_gens = {}) const;
    };

    struct SubGroup : public Group {
        std::vector<int> gen_map;
        const Group &parent;

        SubGroup(const Group &parent, std::vector<int> gen_map)
            : Group(gen_map.size()), parent(parent), gen_map() {

            std::sort(gen_map.begin(), gen_map.end());
            this->gen_map = gen_map;

            for (size_t i = 0; i < gen_map.size(); ++i) {
                for (size_t j = 0; j < gen_map.size(); ++j) {
                    int mult = parent.get(gen_map[i], gen_map[j]);
                    set(Rel(i, j, mult));
                }
            }
        }
    };
}
