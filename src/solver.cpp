#include "solver.h"

#include <algorithm>
#include <functional>

namespace tc {
    Cosets solve(const Group &group, const std::vector<int> &sub_gens) {
        std::vector<int> init_row(group.ngens, -1);
        for (int i : sub_gens) {
            init_row[i] = 0;
        }

        Cosets cosets(group.ngens, init_row);
        std::vector<RelTable> rel_tables;
        std::vector<std::vector<int>> gen_map(group.ngens);
        int rel_idx;
        for (Mult m : group.get_mults()) {
            rel_idx = rel_tables.size();
            gen_map[m.gen0].push_back(rel_idx);
            gen_map[m.gen1].push_back(rel_idx);
            rel_tables.emplace_back(m);
        }

        int null_lst_ptr;
        for (RelTable &rel : rel_tables) {
            int idx = rel.add_row();

            if (cosets.get(rel.gens[0]) + cosets.get(rel.gens[1]) == -2) {
                rel.lst_ptr[idx] = new int;
                rel.gen[idx] = 0;
            } else {
                rel.lst_ptr[idx] = &null_lst_ptr;
                rel.gen[idx] = -1;
            }
        }

        int idx = 0;
        int coset, gen, target, fact_idx, lst, gen_;
        while (true) {
            while (idx < cosets.data.size() and cosets.get(idx) >= 0)
                idx++;

            if (idx == cosets.data.size())
                break;

            target = cosets.len;
            cosets.add_row();

            for (RelTable &rel : rel_tables) {
                rel.add_row();
            }

            std::vector<int> facts;
            facts.push_back(idx);

            coset = idx / group.ngens;
            gen = idx % group.ngens;

            while (!facts.empty()) {
                fact_idx = facts.back();
                facts.pop_back();

                if (cosets.get(fact_idx) != -1)
                    continue;

                cosets.put(fact_idx, target);

                coset = fact_idx / group.ngens;
                gen = fact_idx % group.ngens;

                for (int rel_idx : gen_map[gen]) {
                    RelTable &rel = rel_tables[rel_idx];
                    if (rel.lst_ptr[target] == nullptr) {
                        rel.lst_ptr[target] = rel.lst_ptr[coset];
                        rel.gen[target] = rel.gen[coset] + 1;

                        if (rel.gen[coset] < 0)
                            rel.gen[target] -= 2;

                        if (rel.gen[target] == rel.mult) {
                            lst = *(rel.lst_ptr[target]);
                            delete rel.lst_ptr[target];
                            gen_ = rel.gens[(int) (rel.gens[0] == gen)];
                            facts.push_back(lst * group.ngens + gen_);
                        } else if (rel.gen[target] == -rel.mult) {
                            gen_ = rel.gens[rel.gens[0] == gen];
                            facts.push_back(target * group.ngens + gen_);
                        } else if (rel.gen[target] == rel.mult - 1) {
                            *(rel.lst_ptr[target]) = target;
                        }
                    }
                }

                std::sort(facts.begin(), facts.end(), std::greater<>());
            }

            for (RelTable &rel : rel_tables) {
                if (rel.lst_ptr[target] == nullptr) {
                    if ((cosets.get(target, rel.gens[0]) != target) and
                        (cosets.get(target, rel.gens[1]) != target)) {
                        rel.lst_ptr[target] = new int;
                        rel.gen[target] = 0;
                    } else {
                        rel.lst_ptr[target] = &null_lst_ptr;
                        rel.gen[target] = -1;
                    }
                }
            }
        }

        return cosets;
    }
}