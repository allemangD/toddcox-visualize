#include "solver.h"

#include <algorithm>

namespace tc {
    struct RelTable {
        Rel rel;

        std::vector<int *> lst_ptr;
        std::vector<int> gen;

        int &mult = rel.mult;
        std::array<int, 2> &gens = rel.gens;

        explicit RelTable(Rel rel) : rel(rel) {
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
            const std::vector<Rel> &rels = g.get_rels();
            tables.reserve(rels.size());
            for (const auto &rel : rels) {
                RelTable &table = tables.emplace_back(rel);
                gen_map[rel.gens[0]].push_back(&table);
                gen_map[rel.gens[1]].push_back(&table);
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

                if ((cosets.get(idx, table.gens[0]) != idx) and
                    (cosets.get(idx, table.gens[1]) != idx)) {
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

                for (RelTable *pTable : rels.gen_map[gen]) {
                    RelTable &table = *pTable;

                    if (table.lst_ptr[target] == nullptr) {
                        table.lst_ptr[target] = table.lst_ptr[coset];
                        table.gen[target] = table.gen[coset] + 1;

                        if (table.gen[coset] < 0)
                            table.gen[target] -= 2;

                        if (table.gen[target] == table.rel.mult) {
                            lst = *(table.lst_ptr[target]);
                            delete table.lst_ptr[target];
                            gen_ = table.gens[table.gens[0] == gen];
                            facts.push_back(lst * group.ngens + gen_);
                        } else if (table.gen[target] == -table.mult) {
                            gen_ = table.gens[table.gens[0] == gen];
                            facts.push_back(target * group.ngens + gen_);
                        } else if (table.gen[target] == table.mult - 1) {
                            *(table.lst_ptr[target]) = target;
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