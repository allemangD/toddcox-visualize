#include <tc/core.hpp>

#include <algorithm>

namespace tc {
    SubGroup Group::subgroup(const std::vector<int> &gens) const {
        return {*this, gens};
    }

    struct Row {
        std::vector<int> gnrs;
        std::vector<int *> lst_ptrs;

        Row(int num_tables)
            : gnrs(num_tables, 0), lst_ptrs(num_tables, nullptr) {
        }
    };

    struct Tables {
        std::vector<Rel> rels;
        std::vector<Row> rows;
        int num_tables;

        explicit Tables(const std::vector<Rel> &rels)
            : num_tables(rels.size()), rels(rels) {
        }

        void add_row() {
            rows.emplace_back(num_tables);
        }
    };

    Cosets solve(const Group &group, const std::vector<size_t> &sub_gens) {
        auto ngens = group.ngens;
        Cosets cosets(ngens);
        cosets.add_row();

        if (ngens == 0) {
            return cosets;
        }

        for (int g: sub_gens) {
            if (g < ngens)
                cosets.put(0, g, 0);
        }

        auto rels = group.rels();
        Tables rel_tables(rels);
        std::vector<std::vector<int>> gen_map(ngens);
        int rel_idx = 0;
        for (Rel m: rels) {
            gen_map[m.gens[0]].push_back(rel_idx);
            gen_map[m.gens[1]].push_back(rel_idx);
            rel_idx++;
        }

        int null_lst_ptr;
        rel_tables.add_row();
        Row &row = rel_tables.rows[0];
        for (int table_idx = 0; table_idx < rel_tables.num_tables; table_idx++) {
            Rel &ti = rel_tables.rels[table_idx];

            if (cosets.get(ti.gens[0]) + cosets.get(ti.gens[1]) == -2) {
                row.lst_ptrs[table_idx] = new int;
                row.gnrs[table_idx] = 0;
            } else {
                row.lst_ptrs[table_idx] = &null_lst_ptr;
                row.gnrs[table_idx] = -1;
            }
        }

        int idx = 0;
        int coset, gen, target, fact_idx, lst, gen_;
        while (true) {
            while (idx < cosets.data.size() and cosets.get(idx) >= 0)
                idx++;

            if (idx == cosets.data.size()) {
//                rel_tables.del_rows_to(idx / ngens);
                break;
            }

            target = cosets.size();
            cosets.add_row();
            rel_tables.add_row();

            std::vector<int> facts;
            facts.push_back(idx);

            coset = idx / ngens;
            gen = idx % ngens;

//            rel_tables.del_rows_to(coset);

            Row &target_row = rel_tables.rows[target];
            while (!facts.empty()) {
                fact_idx = facts.back();
                facts.pop_back();

                if (cosets.get(fact_idx) != -1)
                    continue;

                cosets.put(fact_idx, target);

                coset = fact_idx / ngens;
                gen = fact_idx % ngens;

                if (target == coset)
                    for (int table_idx: gen_map[gen])
                        if (target_row.lst_ptrs[table_idx] == nullptr)
                            target_row.gnrs[table_idx] = -1;

                Row &coset_row = rel_tables.rows[coset];
                for (int table_idx: gen_map[gen]) {
                    if (target_row.lst_ptrs[table_idx] == nullptr) {
                        Rel &ti = rel_tables.rels[table_idx];
                        target_row.lst_ptrs[table_idx] = coset_row.lst_ptrs[table_idx];
                        target_row.gnrs[table_idx] = coset_row.gnrs[table_idx] + 1;

                        if (coset_row.gnrs[table_idx] < 0)
                            target_row.gnrs[table_idx] -= 2;

                        if (target_row.gnrs[table_idx] == ti.mult) {
                            lst = *(target_row.lst_ptrs[table_idx]);
                            delete target_row.lst_ptrs[table_idx];
                            gen_ = ti.gens[(int) (ti.gens[0] == gen)];
                            facts.push_back(lst * ngens + gen_);
                        } else if (target_row.gnrs[table_idx] == -ti.mult) {
                            gen_ = ti.gens[ti.gens[0] == gen];
                            facts.push_back(target * ngens + gen_);
                        } else if (target_row.gnrs[table_idx] == ti.mult - 1) {
                            *(target_row.lst_ptrs[table_idx]) = target;
                        }
                    }
                }

                std::sort(facts.begin(), facts.end(), std::greater<>());
            }

            for (int table_idx = 0; table_idx < rel_tables.num_tables; table_idx++) {
                Rel &ti = rel_tables.rels[table_idx];
                if (target_row.lst_ptrs[table_idx] == nullptr) {
                    if ((cosets.get(target, ti.gens[0]) != target) and
                        (cosets.get(target, ti.gens[1]) != target)) {
                        target_row.lst_ptrs[table_idx] = new int;  // todo slow; memory leak.
                        target_row.gnrs[table_idx] = 0;
                    } else {
                        target_row.lst_ptrs[table_idx] = &null_lst_ptr;
                        target_row.gnrs[table_idx] = -1;
                    }
                }
            }
        }

        return cosets;
    }
}
