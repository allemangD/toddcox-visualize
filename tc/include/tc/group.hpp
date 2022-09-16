#pragma once

#include <sstream>

#include <tc/pair_map.hpp>

namespace tc {
    struct Group;
    struct SubGroup;

    /** 
     * @brief Manage the presentation of a Coxeter group and enforce constraints
     * on the multiplicities of its relations.
     * <ul>
     *   <li>
     *     <code>m_ij = 1</code> iff <code>i != j</code>
     *   </li>
     *   <li>
     *     <code>m_ij = m_ji</code>
     *   </li>
     *   <li>
     *     If <code>m_ij == inf</code> (<code>tc::FREE</code>) then no relation is imposed.
     *   </li>
     * </ul>
     * @see
     * <a href="https://en.wikipedia.org/wiki/Coxeter_group#Definition">Coxeter Group (Wikipedia)</a>
     */
    struct Group {
        int ngens;
        tc::pair_map<int> _mults;

        Group(const Group &) = default;

        explicit Group(int ngens, const std::vector<Rel> &rels = {})
            : ngens(ngens), _mults(ngens, 2) {

            for (int i = 0; i < ngens; ++i) {
                set(Rel{i, i, 1});
            }
            
            for (const auto &rel: rels) {
                set(rel);
            }
        }

        void set(const Rel &r) {
            auto &[i, j, m] = r;
            if (i == j && m != 1) {
                throw std::runtime_error("Coxeter groups must satisfy m_ii=1.");
            }
            _mults(i, j) = m;
        }

        [[nodiscard]] int get(int i, int j) const {
            return _mults(i, j);
        }

        [[nodiscard]] SubGroup subgroup(const std::vector<int> &gens) const;
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
