#include <vector>
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
    std::vector<int> fam, gen, lst;
    
    RelTable(Mult m): mult(m.mult) {
        gens[0] = m.gen0;
        gens[1] = m.gen1;
    }

    int add_row() {
        int idx = fam.size();
        fam.push_back(-1);
        gen.push_back(-1);
        lst.push_back(-1);
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
        for (Mult m : get_mults()) {
            rel_tables.emplace_back(m);
        }

        for (RelTable &rel : rel_tables) {
            int idx = rel.add_row();
            
            rel.fam[idx] = 0;
            rel.gen[idx] = 0;
            rel.lst[idx] = 0;

            if ( (cosets.get(rel.gens[0]) == 0) xor (cosets.get(rel.gens[1]) == 0) ) {
                rel.gen[idx] = -1;
            }
        }

        int idx = 0;
        while (true) {
            while (idx < cosets.data.size() and cosets.get(idx) >= 0)
                idx++;

            if (idx == cosets.data.size())
                break;

            int coset = idx / ngens;
            int gen = idx % ngens;
            int target = cosets.len;

            cosets.add_row();
            
            for (RelTable &rel : rel_tables) {
                rel.add_row();
            }

            std::priority_queue<int, std::vector<int>, std::greater<int>> facts;
            facts.push(coset*ngens + gen);

            while (!facts.empty()) {
                int fact_idx = facts.top();
                facts.pop();

                if (cosets.get(fact_idx) != -1)
                    continue;

                cosets.put(fact_idx, target);

                coset = fact_idx / ngens;
                gen = fact_idx % ngens;


                for (RelTable &rel : rel_tables) {
                    if ( (gen == rel.gens[0] or gen == rel.gens[1]) and (rel.fam[target] == -1) ) {
                        rel.fam[target] = rel.fam[coset];
                        rel.gen[target] = rel.gen[coset] + 1;
                        
                        if (rel.gen[coset] < 0)
                            rel.gen[target] -= 2;

                        if (rel.gen[target] == rel.mult) {
                            int lst = rel.lst[rel.fam[target]];
                            int gen_ = rel.gens[(int)(rel.gens[0] == gen)];
                            facts.push(lst*ngens + gen_);
                        }
                        else if (rel.gen[target] == -rel.mult) {
                            int gen_ = rel.gens[rel.gens[0] == gen];
                            facts.push(target*ngens + gen_);
                        }
                        else if (rel.gen[target] == rel.mult - 1) {
                            rel.lst[rel.fam[target]] = target;
                        }
                    }
                }
            }

            for (RelTable &rel : rel_tables) {
                if (rel.fam[target] == -1) {
                    rel.fam[target] = target;
                    rel.gen[target] = 0;

                    if ( (cosets.get(target, rel.gens[0]) == target) xor (cosets.get(target, rel.gens[1]) == target) )
                        rel.gen[target] = -1;
                }
            }
        }
        
        return cosets;
    }
};
