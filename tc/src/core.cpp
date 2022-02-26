#include <tc/core.hpp>

#include <algorithm>

namespace tc {
    SubGroup Group::subgroup(const std::vector<int> &gens) const {
        return {*this, gens};
    }

    struct Row {
        int gnr = 0;
        int *lst_ptr = nullptr;
    };

    struct Tables {
        std::vector<Rel> rels;
        std::vector<std::vector<Row>> cols;

        explicit Tables(const std::vector<Rel> &rels)
            : rels(rels), cols(rels.size()) {
        }

        [[nodiscard]] size_t size() const {
            return rels.size();
        }

        void add_row() {
            for (auto &col: cols) {
                col.emplace_back();
            }
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
        for (int table_idx = 0; table_idx < rel_tables.size(); ++table_idx) {
            Rel &rel = rel_tables.rels[table_idx];
            Row &row = rel_tables.cols[table_idx][0];

            if (cosets.get(rel.gens[0]) + cosets.get(rel.gens[1]) == -2) {
                row.lst_ptr = new int;
                row.gnr = 0;
            } else {
                row.lst_ptr = &null_lst_ptr;
                row.gnr = -1;
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

            while (!facts.empty()) {
                fact_idx = facts.back();
                facts.pop_back();

                if (cosets.get(fact_idx) != -1)
                    continue;

                cosets.put(fact_idx, target);

                coset = fact_idx / ngens;
                gen = fact_idx % ngens;

                if (target == coset)
                    for (int table_idx: gen_map[gen]) {
                        auto &col = rel_tables.cols[table_idx];
                        auto &row = col[target];

                        if (row.lst_ptr == nullptr) {
                            row.gnr = -1;
                        }
                    }

                for (int table_idx: gen_map[gen]) {
                    auto &col = rel_tables.cols[table_idx];
                    auto &trow = col[target];
                    auto &crow = col[coset];

                    if (trow.lst_ptr == nullptr) {
                        Rel &ti = rel_tables.rels[table_idx];
                        trow.lst_ptr = crow.lst_ptr;
                        trow.gnr = crow.gnr + 1;

                        if (crow.gnr < 0)
                            trow.gnr -= 2;

                        if (trow.gnr == ti.mult) {
                            lst = *(trow.lst_ptr);
                            delete trow.lst_ptr;
                            gen_ = ti.gens[(int) (ti.gens[0] == gen)];
                            facts.push_back(lst * ngens + gen_);
                        } else if (trow.gnr == -ti.mult) {
                            gen_ = ti.gens[ti.gens[0] == gen];
                            facts.push_back(target * ngens + gen_);
                        } else if (trow.gnr == ti.mult - 1) {
                            *(trow.lst_ptr) = target;
                        }
                    }
                }

                std::sort(facts.begin(), facts.end(), std::greater<>());
            }

            for (int table_idx = 0; table_idx < rel_tables.size(); table_idx++) {
                auto &rel = rel_tables.rels[table_idx];
                auto &col = rel_tables.cols[table_idx];
                auto &trow = col[target];

                if (trow.lst_ptr == nullptr) {
                    if ((cosets.get(target, rel.gens[0]) != target) and
                        (cosets.get(target, rel.gens[1]) != target)) {
                        trow.lst_ptr = new int;  // todo slow; memory leak.
                        trow.gnr = 0;
                    } else {
                        trow.lst_ptr = &null_lst_ptr;
                        trow.gnr = -1;
                    }
                }
            }
        }

        return cosets;
    }
}
