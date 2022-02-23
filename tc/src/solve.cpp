#include "tc/core.hpp"

#include <algorithm>

namespace tc {
    struct RelTablesRow {
        int *gnrs;
        int **lst_ptrs;

        RelTablesRow(int N, int *gnrs, int **lst_ptrs) : gnrs(gnrs), lst_ptrs(lst_ptrs) {
            for (int i = 0; i < N; i++) {
                lst_ptrs[i] = nullptr;
            }
        }
    };

    struct RelTables {
        static const int ROW_BLOCK_SIZE = 64;
        std::vector<Rel> rels;
        std::vector<RelTablesRow *> rows;
        int start = 0;
        int num_tables;
        int buffer_rows = 0;

        explicit RelTables(const std::vector<Rel> &rels)
            : num_tables(rels.size()), rels(rels) {
        }

        void add_row() {
            if (buffer_rows == 0) {
                int *gnrs_alloc = new int[num_tables * RelTables::ROW_BLOCK_SIZE];
                int **lst_ptrs_alloc = new int *[num_tables * RelTables::ROW_BLOCK_SIZE];
                for (int i = 0; i < RelTables::ROW_BLOCK_SIZE; i++) {
                    rows.push_back(
                        new RelTablesRow(num_tables, &gnrs_alloc[i * num_tables], &lst_ptrs_alloc[i * num_tables]));
                }
                buffer_rows = RelTables::ROW_BLOCK_SIZE;
            }

            buffer_rows--;
        }

        void del_rows_to(int idx) {
            const int del_to = (idx / RelTables::ROW_BLOCK_SIZE) * RelTables::ROW_BLOCK_SIZE;
            for (int i = start; i < del_to; i += RelTables::ROW_BLOCK_SIZE) {
                delete[] rows[i]->gnrs;
                delete[] rows[i]->lst_ptrs;
                for (int j = 0; j < RelTables::ROW_BLOCK_SIZE; j++) {
                    delete rows[i + j];
                }
                start += RelTables::ROW_BLOCK_SIZE;
            }
        }

        ~RelTables() {
            while (start < rows.size()) {
                delete[] rows[start]->gnrs;
                delete[] rows[start]->lst_ptrs;
                for (int j = 0; j < RelTables::ROW_BLOCK_SIZE; j++) {
                    delete rows[start + j];
                }
                start += RelTables::ROW_BLOCK_SIZE;
            }
        }
    };

    Cosets Group::solve(const std::vector<int> &sub_gens) const {
        Cosets cosets(ngens);
        cosets.add_row();

        if (ngens == 0) {
            return cosets;
        }

        for (int g : sub_gens) {
            if (g < ngens)
                cosets.put(0, g, 0);
        }

        RelTables rel_tables(rels());
        std::vector<std::vector<int>> gen_map(ngens);
        int rel_idx = 0;
        for (Rel m : rels()) {
            gen_map[m.gens[0]].push_back(rel_idx);
            gen_map[m.gens[1]].push_back(rel_idx);
            rel_idx++;
        }

        int null_lst_ptr;
        rel_tables.add_row();
        RelTablesRow &row = *(rel_tables.rows[0]);
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
                rel_tables.del_rows_to(idx / ngens);
                break;
            }

            target = cosets.size();
            cosets.add_row();
            rel_tables.add_row();

            std::vector<int> facts;
            facts.push_back(idx);

            coset = idx / ngens;
            gen = idx % ngens;

            rel_tables.del_rows_to(coset);

            RelTablesRow &target_row = *(rel_tables.rows[target]);
            while (!facts.empty()) {
                fact_idx = facts.back();
                facts.pop_back();

                if (cosets.get(fact_idx) != -1)
                    continue;

                cosets.put(fact_idx, target);

                coset = fact_idx / ngens;
                gen = fact_idx % ngens;

                if (target == coset)
                    for (int table_idx : gen_map[gen])
                        if (target_row.lst_ptrs[table_idx] == nullptr)
                            target_row.gnrs[table_idx] = -1;

                RelTablesRow &coset_row = *(rel_tables.rows[coset]);
                for (int table_idx : gen_map[gen]) {
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
                        target_row.lst_ptrs[table_idx] = new int;
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
