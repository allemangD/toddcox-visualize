#include <tc/core.hpp>

#include <algorithm>
#include <queue>
#include <utility>
#include <vector>

namespace tc {
    SubGroup Group::subgroup(const std::vector<int> &gens) const {
        return {*this, gens};
    }

    /**
     * Each coset is associated a row in each table.
     * Rows document the "loops" formed by 
     */
    struct Row {
        bool free: 1;
        bool idem: 1;
        unsigned int gnr: 14;  // progress through the loop
        unsigned int lst_idx: 32;  // the coset that would complete the loop

        Row() : free(true), idem(false), gnr(0), lst_idx(0) {}
    };

    struct Tables {
        std::vector<Rel> rels;
        std::vector<std::vector<Row>> rows;

        explicit Tables(std::vector<Rel> rels)
            : rels(std::move(rels)), rows() {
        }

        [[nodiscard]] size_t size() const {
            return rels.size();
        }

        void add_row() {
            rows.emplace_back(rels.size());
        }
    };

    Cosets solve(
        const Group &group,
        const std::vector<Gen> &sub_gens,
        const Coset &bound
    ) {
        auto rank = group.rank;

        // region Initialize Cosets Table
        Cosets cosets(rank);
        cosets.add_row();

        if (rank == 0) {
            cosets.complete = true;
            return cosets;
        }

        for (Coset g: sub_gens) {
            if (g < rank)
                cosets.put(0, g, 0);
        }
        // endregion

        // region Initialize Relation Tables
        std::vector<std::tuple<Gen, Gen, Mult>> rels;
        for (const auto &[i, j, m]: group._orders) {
            // The algorithm only works for Coxeter groups; multiplicities m_ii=1 are assumed. Relation tables
            // _may_ be added for them, but they are redundant and hurt performance so are skipped.
            if (i == j) continue;

            // Coxeter groups admit infinite multiplicities, represented by contexpr tc::FREE. Relation tables
            // for these should be skipped.
            if (m == FREE) {
                continue;
            }

            rels.emplace_back(i, j, m);
        }

        Tables rel_tables(rels);
        std::vector<std::vector<size_t>> tables_for(rank);
        int rel_idx = 0;
        for (const auto &[i, j, m]: rels) {
            tables_for[i].push_back(rel_idx);
            tables_for[j].push_back(rel_idx);
            rel_idx++;
        }

        std::vector<Coset> lst_vals;
        rel_tables.add_row();
        for (int table_idx = 0; table_idx < rel_tables.size(); ++table_idx) {
            const auto &[i, j, m] = rel_tables.rels[table_idx];
            Row &row = rel_tables.rows[0][table_idx];

            if (!cosets.isset(0, i) && !cosets.isset(0, j)) {
                row.lst_idx = lst_vals.size();
                lst_vals.push_back(0);
                row.free = false;
                row.gnr = 0;
            } else {
                row.free = false;
                row.gnr = 1;
                row.idem = true;
            }
        }
        // endregion

        size_t idx = 0;
        size_t fact_idx;
        Coset coset, gen, target, lst;

        while (true) {
            // find next unknown product
            while (idx < cosets.data.size() and cosets.isset(idx))
                idx++;

            if (cosets.size() >= bound) {
                return cosets;
            }

            // if there are none, then return
            if (idx == cosets.data.size()) {
                // todo unrolled linked list interval
//                rel_tables.del_rows_to(idx / ngens);  
                break;
            }

            // the unknown product must be a new coset, so add it
            target = cosets.size();
            cosets.add_row();
            rel_tables.add_row();

            // queue of products that equal target
            std::queue<size_t> facts;
            facts.push(idx);  // new product should be recorded and propagated

            // todo unrolled linked list interval
//            coset = idx / ngens;
//            gen = idx % ngens;
//            rel_tables.del_rows_to(coset);

            // find all products which also lead to target
            while (!facts.empty()) {
                fact_idx = facts.front();
                facts.pop();

                // skip if this product was already learned
                if (cosets.get(fact_idx) != -1) continue;

                cosets.put(fact_idx, target);

                coset = fact_idx / rank;
                gen = fact_idx % rank;

                // If the product stays within the coset todo
                for (size_t table_idx: tables_for[gen]) {
                    auto &[i, j, m] = rel_tables.rels[table_idx];
                    auto &trow = rel_tables.rows[target][table_idx];
                    auto &crow = rel_tables.rows[coset][table_idx];

                    Coset other_gen = (i == gen) ? j : i;

                    // Test if loop is closed
                    if (trow.free) {
                        trow = crow;
                        trow.gnr++;

                        if (target == coset) {
                            trow.idem = true;
                        }

                        if (trow.idem) {
                            if (trow.gnr == m) {
                                // loop is closed, but idempotent, so the target links to itself via the other generator.
                                // todo might be able to move this logic up into the (target == coset) block and avoid those computations.
                                facts.push(target * rank + other_gen);
                            }
                        } else {
                            if (trow.gnr == m - 1) {
                                // loop is almost closed. record that the target closes this loop.
                                lst_vals[trow.lst_idx] = target;
                            } else if (trow.gnr == m) {
                                // loop is closed. We know the last element in the loop must link with this one. 
                                lst = lst_vals[trow.lst_idx];
//                            delete trow.lst_ptr;
                                facts.push(lst * rank + other_gen);
                            }
                        }
                    }
                }
            }

            // If any target row wasn't identified with a loop,
            // then assign it a new loop.
            for (size_t table_idx = 0; table_idx < rel_tables.size(); table_idx++) {
                auto &[i, j, m] = rel_tables.rels[table_idx];
                auto &trow = rel_tables.rows[target][table_idx];

                if (trow.free) {
                    if ((cosets.get(target, i) != target) and
                        (cosets.get(target, j) != target)) {
                        trow.lst_idx = lst_vals.size();
                        trow.free = false;
                        lst_vals.push_back(0);
                        trow.gnr = 0;
                    } else {
                        trow.free = false;
                        trow.gnr = 1;
                        trow.idem = true;
                    }
                }
            }
        }

        cosets.complete = true;
        return cosets;
    }
}
