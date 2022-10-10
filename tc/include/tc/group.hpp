#pragma once

#include <cassert>
#include <sstream>

#include <tc/util.hpp>
#include <tc/pair_map.hpp>

namespace tc {
    struct Group;
    struct SubGroup;

    struct Graph {
        size_t rank{};
        std::vector<tc::Rel> edges{};
    };

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
        int rank;
        tc::pair_map<int> _orders;

        Group(const Group &) = default;

        explicit Group(int rank, const std::vector<Rel> &rels = {})
            : rank(rank), _orders(rank, 2) {

            for (int i = 0; i < rank; ++i) {
                set(Rel{i, i, 1});
            }

            for (const auto &rel: rels) {
                set(rel);
            }
        }

        explicit Group(const Graph &graph)
            : rank(graph.rank), _orders(graph.rank, 2) {
            for (const auto &[i, j, order]: graph.edges) {
                set({i, j, order});
            }
        }

        void set(const Rel &r) {
            auto &[i, j, m] = r;
            assert(i != j || m == 1);
            _orders(i, j) = m;
        }

        [[nodiscard]] int get(int i, int j) const {
            return _orders(i, j);
        }

        [[nodiscard]] SubGroup subgroup(const std::vector<tc::Gen> &gens) const;
    };

    struct SubGroup : public Group {
        std::vector<tc::Gen> gen_map;
        const Group &parent;

        SubGroup(const Group &parent, std::vector<tc::Gen> gen_map)
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
