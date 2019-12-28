#include "solver.h"

#include <algorithm>

namespace tc {
    struct RelTable {
        Mult mult;

        std::vector<int *> lst_ptr;
        std::vector<int> gen;

        explicit RelTable(tc::Mult mult) : mult(mult) {
        }

        int add_row() {
            int idx = lst_ptr.size();
            lst_ptr.push_back(nullptr);
            gen.push_back(-1);
            return idx;
        }
    };

    struct RelationSet {
        const Cosets &cosets;

        std::vector<RelTable> tables;
        std::vector<std::vector<RelTable *>> gen_map; // which relations involve which generators

        explicit RelationSet(const Group &g, const Cosets &cosets) : gen_map(g.ngens), cosets(cosets) {
            const std::vector<Mult> &mults = g.get_mults();
            tables.reserve(mults.size());
            for (const auto &mult : mults) {
                RelTable &table = tables.emplace_back(mult);
                gen_map[mult.gens[0]].push_back(&table);
                gen_map[mult.gens[1]].push_back(&table);
            }
        }

        void add_row() {
            for (auto &table : tables) {
                table.add_row();
            }
        }

        void fill_row(int idx) {
            for (auto &table : tables) {
                if (table.lst_ptr[idx] != nullptr) continue;

                table.lst_ptr[idx] = new int;

                if ((cosets.get(idx, table.mult.gens[0]) != idx) and
                    (cosets.get(idx, table.mult.gens[1]) != idx)) {
                    table.gen[idx] = 0;
                } else {
                    table.gen[idx] = -1;
                }
            }
        }
    };

    Cosets solve(const Group &group, const std::vector<int> &sub_gens) {
        Cosets cosets(group.ngens);
        cosets.add_row();
        for (const auto &i : sub_gens) {
            cosets.put(0, i, 0);
        }

        RelationSet rels(group, cosets);
        rels.add_row();
        rels.fill_row(0);

        int idx = 0;
        int coset, gen, target, fact_idx, lst, gen_;
        while (true) {
            while (idx < cosets.data.size() and cosets.get(idx) >= 0)
                idx++;

            if (idx == cosets.data.size())
                break;

            target = cosets.len;

            cosets.add_row();
            rels.add_row();

            std::vector<int> facts = {idx};

            while (!facts.empty()) {
                fact_idx = facts.back();
                facts.pop_back();

                if (cosets.get(fact_idx) != -1)
                    continue;

                cosets.put(fact_idx, target);

                coset = fact_idx / group.ngens;
                gen = fact_idx % group.ngens;

                for (RelTable *rel : rels.gen_map[gen]) {
                    if (rel->lst_ptr[target] == nullptr) {
                        rel->lst_ptr[target] = rel->lst_ptr[coset];
                        rel->gen[target] = rel->gen[coset] + 1;

                        if (rel->gen[coset] < 0)
                            rel->gen[target] -= 2;

                        if (rel->gen[target] == rel->mult.mult) {
                            lst = *(rel->lst_ptr[target]);
                            delete rel->lst_ptr[target];
                            gen_ = rel->mult.gens[(int) (rel->mult.gens[0] == gen)];
                            facts.push_back(lst * group.ngens + gen_);
                        } else if (rel->gen[target] == -rel->mult.mult) {
                            gen_ = rel->mult.gens[rel->mult.gens[0] == gen];
                            facts.push_back(target * group.ngens + gen_);
                        } else if (rel->gen[target] == rel->mult.mult - 1) {
                            *(rel->lst_ptr[target]) = target;
                        }
                    }
                }

                std::sort(facts.begin(), facts.end(), std::greater<>());
            }

            rels.fill_row(target);
        }

        return cosets;
    }
}