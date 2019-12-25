#include <vector>
#include <algorithm>
#include <queue>
#include <utility>
#include <functional>
#include <iostream>

struct Cosets {
    int ngens;
    std::vector<int> data;
    int len;

    Cosets(int ngens, std::vector<int> data): ngens(ngens), data(data) {
        len = data.size() / ngens;
    }

    void add_row() {
        len++;
        data.resize(data.size()+ngens, -1);
    }

    void put(int coset, int gen, int target) {
        data[coset * ngens + gen] = target;
        data[target * ngens + gen] = coset;
    }

    void put(int idx, int target) {
        int coset = idx / ngens;
        int gen = idx % ngens;
        data[idx] = target;
        data[target * ngens + gen] = coset;
    }

    int get(int coset, int gen) {
        return data[coset * ngens + gen];
    }

    int get(int idx) {
        return data[idx];
    }
};

struct Mult {
    int gen0, gen1, mult;
};

struct RelTable {
    int gens[2];
    int mult;
    std::vector<int*> lst_ptr;
    std::vector<int> gen;
    
    RelTable(Mult m): mult(m.mult) {
        gens[0] = m.gen0;
        gens[1] = m.gen1;
    }

    int add_row() {
        int idx = lst_ptr.size();
        lst_ptr.push_back(nullptr);
        gen.push_back(-1);
        return idx;
    }
};

struct Group {
    int ngens;
    std::vector<std::vector<int>> _mults;

    Group(int ngens, std::vector<Mult> rels = {}): ngens(ngens) {
        _mults.resize(ngens);
        for (int i = 0; i < ngens; i++) {
            _mults[i].resize(ngens, 2);
        }

        for (Mult m : rels) {
            if (m.gen0 < m.gen1)
                _mults[m.gen0][m.gen1] = m.mult;
            else
                _mults[m.gen1][m.gen0] = m.mult;
        }
    }

    void setmult(Mult m) {
        if (m.gen0 < m.gen1)
            _mults[m.gen0][m.gen1] = m.mult;
        else
            _mults[m.gen1][m.gen0] = m.mult;
    }

    std::vector<Mult> get_mults() const {
        std::vector<Mult> mults; 
        for (int i = 0; i < ngens - 1; i++) {
            for (int j = i+1; j < ngens; j++) {
                mults.push_back({i,j,_mults[i][j]});
            }
        }
        return mults;
    }

    Group operator*(const Group &other) {
        int off = ngens;

        Group g(ngens + other.ngens, get_mults());
        
        for (Mult m : other.get_mults()) {
            g.setmult({off + m.gen0, off + m.gen1, m.mult});
        }

        return g;
    }

    Group operator^(int p) {
        Group g(ngens * p);

        for (Mult m : get_mults()) {
            for (int off = 0; off < g.ngens; off += ngens) {
                g.setmult({off + m.gen0, off + m.gen1, m.mult});
            }
        }

        return g;
    }

    static Group schlafli(std::vector<int> mults) {
        int ngens = mults.size() + 1;
        Group g(ngens);
        for (int i = 0; i < mults.size(); i++) {
            g.setmult({i, i+1, mults[i]});
        }
        return g;
    }

    Cosets solve(std::vector<int> sub_gens = {}) {
        std::vector<int> init_row(ngens, -1);
        for (int i : sub_gens) {
            init_row[i] = 0;
        }

        Cosets cosets(ngens, init_row);
        std::vector<RelTable> rel_tables;
        std::vector<std::vector<int>> gen_map(ngens);
        int rel_idx;
        for (Mult m : get_mults()) {
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
            }
            else {
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

            coset = idx / ngens;
            gen = idx % ngens;

            while (!facts.empty()) {
                fact_idx = facts.back();
                facts.pop_back();

                if (cosets.get(fact_idx) != -1)
                    continue;

                cosets.put(fact_idx, target);

                coset = fact_idx / ngens;
                gen = fact_idx % ngens;

                for (int rel_idx : gen_map[gen]) {
                    RelTable &rel = rel_tables[rel_idx];
                    if ( rel.lst_ptr[target] == nullptr ) {
                        rel.lst_ptr[target] = rel.lst_ptr[coset];
                        rel.gen[target] = rel.gen[coset] + 1;
                        
                        if (rel.gen[coset] < 0)
                            rel.gen[target] -= 2;

                        if (rel.gen[target] == rel.mult) {
                            lst = *(rel.lst_ptr[target]);
                            delete rel.lst_ptr[target];
                            gen_ = rel.gens[(int)(rel.gens[0] == gen)];
                            facts.push_back(lst*ngens + gen_);
                        }
                        else if (rel.gen[target] == -rel.mult) {
                            gen_ = rel.gens[rel.gens[0] == gen];
                            facts.push_back(target*ngens + gen_);
                        }
                        else if (rel.gen[target] == rel.mult - 1) {
                            *(rel.lst_ptr[target]) = target;
                        }
                    }
                }

                std::sort(facts.begin(), facts.end(), std::greater<int>());
            }

            for (RelTable &rel : rel_tables) {
                if (rel.lst_ptr[target] == nullptr) {
                    if ( (cosets.get(target, rel.gens[0]) != target) and 
                         (cosets.get(target, rel.gens[1]) != target) ) {
                        rel.lst_ptr[target] = new int;
                        rel.gen[target] = 0;
                    }
                    else {
                        rel.lst_ptr[target] = &null_lst_ptr;
                        rel.gen[target] = -1;
                    }
                }
            }
        }
        
        return cosets;
    }
};
