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

    Cosets solve(const Group &group, const std::vector<Coset> &sub_gens) {
        auto ngens = group.ngens;

        // region Initialize Cosets Table
        Cosets cosets(ngens);
        cosets.add_row();

        if (ngens == 0) {
            return cosets;
        }

        for (Coset g: sub_gens) {
            if (g < ngens)
                cosets.put(0, g, 0);
        }
        // endregion

        auto rels = group.rels();

        // region Initialize Relation Tables
        Tables rel_tables(rels);
        std::vector<std::vector<size_t>> tables_for(ngens);
        int rel_idx = 0;
        for (Rel m: rels) {
            tables_for[m.gens[0]].push_back(rel_idx);
            tables_for[m.gens[1]].push_back(rel_idx);
            rel_idx++;
        }

        std::vector<Coset> lst_vals;
        rel_tables.add_row();
        for (int table_idx = 0; table_idx < rel_tables.size(); ++table_idx) {
            Rel &rel = rel_tables.rels[table_idx];
            Row &row = rel_tables.rows[0][table_idx];

            if (cosets.get(rel.gens[0]) + cosets.get(rel.gens[1]) == -2) {
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
            while (idx < cosets.data.size() and cosets.get(idx) >= 0)
                idx++;

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

                coset = fact_idx / ngens;
                gen = fact_idx % ngens;

                // If the product stays within the coset todo
                for (size_t table_idx: tables_for[gen]) {
                    auto &rel = rel_tables.rels[table_idx];
                    auto &trow = rel_tables.rows[target][table_idx];
                    auto &crow = rel_tables.rows[coset][table_idx];

                    // Test if loop is closed
                    Coset other_gen = rel.gens[0] == gen ? rel.gens[1] : rel.gens[0];

                    if (trow.free) {
                        if (target == coset) {
                            trow.gnr = -1;
                        }

                        trow = crow;
                        trow.gnr++;

                        if (trow.idem) {
                            if (trow.gnr == rel.mult) {
                                // loop is closed, but internal, so the target links to itself via this generator.
                                // todo might be able to move this logic up into the (target == coset) block and avoid those computations.
                                facts.push(target * ngens + other_gen);
                            }
                        } else {
                            if (trow.gnr == rel.mult - 1) {
                                // loop is almost closed. record that the target closes this loop.
                                lst_vals[trow.lst_idx] = target;
                            } else if (trow.gnr == rel.mult) {
                                // loop is closed. We know the last element in the loop must link with this one. 
                                lst = lst_vals[trow.lst_idx];
//                            delete trow.lst_ptr;
                                facts.push(lst * ngens + other_gen);
                            }
                        }
                    }
                }
            }

            // If any target row wasn't identified with a loop,
            // then assign it a new loop.
            for (size_t table_idx = 0; table_idx < rel_tables.size(); table_idx++) {
                auto &rel = rel_tables.rels[table_idx];
                auto &trow = rel_tables.rows[target][table_idx];

                if (trow.free) {
                    if ((cosets.get(target, rel.gens[0]) != target) and
                        (cosets.get(target, rel.gens[1]) != target)) {
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

        return cosets;
    }
}
