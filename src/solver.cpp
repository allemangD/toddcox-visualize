#include "solver.h"

#include <algorithm>

namespace tc {
    struct RelationSet {
        std::vector<RelTable> tables;
        std::vector<std::vector<RelTable *>> gen_map; // which relations involve which generators

        explicit RelationSet(const Group &g) : gen_map(g.ngens) {
            const std::vector<Mult> &mults = g.get_mults();
            tables.reserve(mults.size());
            for (const auto &mult : mults) {
                RelTable &table = tables.emplace_back(mult);
                gen_map[mult.gens[0]].push_back(&table);
                gen_map[mult.gens[1]].push_back(&table);
            }
        }
    };

    Cosets solve(const Group &group, const std::vector<int> &sub_gens) {
        std::vector<int> init_row(group.ngens, -1);
        for (int i : sub_gens) {
            init_row[i] = 0;
        }

        Cosets cosets(group.ngens, init_row);
        RelationSet rels(group);

        int null_lst_ptr;
        for (RelTable &rel : rels.tables) {
            int idx = rel.add_row();

            if (cosets.get(rel.mult.gens[0]) + cosets.get(rel.mult.gens[1]) == -2) {
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

            for (RelTable &rel : rels.tables) {
                rel.add_row();
            }

            std::vector<int> facts;
            facts.push_back(idx);

            while (!facts.empty()) {
                fact_idx = facts.back();
                facts.pop_back();

                if (cosets.get(fact_idx) != -1)
                    continue;

                cosets.put(fact_idx, target);

                coset = fact_idx / group.ngens;
                gen = fact_idx % group.ngens;

                for (RelTable *rel_idx : rels.gen_map[gen]) {
                    RelTable &rel = *rel_idx;
                    if (rel.lst_ptr[target] == nullptr) {
                        rel.lst_ptr[target] = rel.lst_ptr[coset];
                        rel.gen[target] = rel.gen[coset] + 1;

                        if (rel.gen[coset] < 0)
                            rel.gen[target] -= 2;

                        if (rel.gen[target] == rel.mult.mult) {
                            lst = *(rel.lst_ptr[target]);
                            delete rel.lst_ptr[target];
                            gen_ = rel.mult.gens[(int) (rel.mult.gens[0] == gen)];
                            facts.push_back(lst * group.ngens + gen_);
                        } else if (rel.gen[target] == -rel.mult.mult) {
                            gen_ = rel.mult.gens[rel.mult.gens[0] == gen];
                            facts.push_back(target * group.ngens + gen_);
                        } else if (rel.gen[target] == rel.mult.mult - 1) {
                            *(rel.lst_ptr[target]) = target;
                        }
                    }
                }

                std::sort(facts.begin(), facts.end(), std::greater<>());
            }

            for (RelTable &rel : rels.tables) {
                if (rel.lst_ptr[target] == nullptr) {
                    if ((cosets.get(target, rel.mult.gens[0]) != target) and
                        (cosets.get(target, rel.mult.gens[1]) != target)) {
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