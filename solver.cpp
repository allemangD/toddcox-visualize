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

struct RelTablesRow {
    int* gnrs;
    int** lst_ptrs;
    RelTablesRow (int N, int* gnrs, int** lst_ptrs): gnrs(gnrs), lst_ptrs(lst_ptrs) {
        for (int i = 0; i < N; i++) {
            lst_ptrs[i] = nullptr;
        }
    }
};

struct TableInfo {
    int gens[2];
    int mult;
    TableInfo(Mult m) {
        gens[0] = m.gen0;
        gens[1] = m.gen1;
        mult = m.mult;
    }
};

struct RelTables {
    static const int ROW_BLOCK_SIZE = 128;
    std::vector<TableInfo> table_info;
    std::vector<RelTablesRow*> rows;
    int start = 0;
    int num_tables;
    int buffer_rows = 0;

    RelTables (std::vector<Mult> mults): num_tables(mults.size()) {
        for (Mult m : mults) {
            table_info.emplace_back(m);
        }
    }

    void add_row() {
        if (buffer_rows == 0) {
            int* gnrs_alloc = new int[num_tables*RelTables::ROW_BLOCK_SIZE];
            int** lst_ptrs_alloc = new int*[num_tables*RelTables::ROW_BLOCK_SIZE];
            for (int i = 0; i < RelTables::ROW_BLOCK_SIZE; i++) {
                rows.push_back(new RelTablesRow(num_tables, &gnrs_alloc[i*num_tables], &lst_ptrs_alloc[i*num_tables]));
            }
            buffer_rows = RelTables::ROW_BLOCK_SIZE;
        }

        buffer_rows--;
    }

    void del_rows_to(int idx) {
        const int del_to = (idx/RelTables::ROW_BLOCK_SIZE)*RelTables::ROW_BLOCK_SIZE;
        for (int i = start; i < del_to; i += RelTables::ROW_BLOCK_SIZE) {
            delete[] rows[i]->gnrs;
            delete[] rows[i]->lst_ptrs;
            for (int j = 0; j < RelTables::ROW_BLOCK_SIZE; j++) {
                delete rows[i+j];
            }
            start += RelTables::ROW_BLOCK_SIZE;
        }
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
        RelTables rel_tables(get_mults());
        std::vector<std::vector<int>> gen_map(ngens);
        int rel_idx = 0;
        for (Mult m : get_mults()) {
            gen_map[m.gen0].push_back(rel_idx);
            gen_map[m.gen1].push_back(rel_idx);
            rel_idx++;
        }

        int null_lst_ptr;
        rel_tables.add_row();
        RelTablesRow &row = *(rel_tables.rows[0]);
        for (int table_idx = 0; table_idx < rel_tables.num_tables; table_idx++) {
            TableInfo &ti = rel_tables.table_info[table_idx];
            
            if (cosets.get(ti.gens[0]) + cosets.get(ti.gens[1]) == -2) {
                row.lst_ptrs[table_idx] = new int;
                row.gnrs[table_idx] = 0;
            }
            else {
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

            target = cosets.len;
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

                RelTablesRow &coset_row = *(rel_tables.rows[coset]);
                for (int table_idx : gen_map[gen]) {
                    if ( target_row.lst_ptrs[table_idx] == nullptr ) {
                        TableInfo &ti = rel_tables.table_info[table_idx];
                        target_row.lst_ptrs[table_idx] = coset_row.lst_ptrs[table_idx];
                        target_row.gnrs[table_idx] = coset_row.gnrs[table_idx] + 1;
                        
                        if (coset_row.gnrs[table_idx] < 0)
                            target_row.gnrs[table_idx] -= 2;

                        if (target_row.gnrs[table_idx] == ti.mult) {
                            lst = *(target_row.lst_ptrs[table_idx]);
                            delete target_row.lst_ptrs[table_idx];
                            gen_ = ti.gens[(int)(ti.gens[0] == gen)];
                            facts.push_back(lst*ngens + gen_);
                        }
                        else if (target_row.gnrs[table_idx] == -ti.mult) {
                            gen_ = ti.gens[ti.gens[0] == gen];
                            facts.push_back(target*ngens + gen_);
                        }
                        else if (target_row.gnrs[table_idx] == ti.mult - 1) {
                            *(target_row.lst_ptrs[table_idx]) = target;
                        }
                    }
                }

                std::sort(facts.begin(), facts.end(), std::greater<int>());
            }

            for (int table_idx = 0; table_idx < rel_tables.num_tables; table_idx++) {
                TableInfo &ti = rel_tables.table_info[table_idx];
                if (target_row.lst_ptrs[table_idx] == nullptr) {
                    if ( (cosets.get(target, ti.gens[0]) != target) and 
                         (cosets.get(target, ti.gens[1]) != target) ) {
                        target_row.lst_ptrs[table_idx] = new int;
                        target_row.gnrs[table_idx] = 0;
                    }
                    else {
                        target_row.lst_ptrs[table_idx] = &null_lst_ptr;
                        target_row.gnrs[table_idx] = -1;
                    }
                }
            }
        }
        
        return cosets;
    }
};
