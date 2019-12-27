#pragma once

#include <vector>

struct Cosets {
    int ngens;
    std::vector<int> data;
    int len;

    Cosets(int ngens, const std::vector<int> &data);

    void add_row();

    void put(int coset, int gen, int target);

    void put(int idx, int target);

    [[nodiscard]] int get(int coset, int gen) const;

    [[nodiscard]] int get(int idx) const;
};

struct Mult {
    int gen0, gen1, mult;
};

struct RelTable {
    int gens[2]{};
    int mult;
    std::vector<int *> lst_ptr;
    std::vector<int> gen;

    explicit RelTable(Mult m);

    int add_row();
};

struct Group {
    int ngens;
    std::vector<std::vector<int>> _mults;

    explicit Group(int ngens, const std::vector<Mult> &rels = {});

    void setmult(Mult m);

    [[nodiscard]] std::vector<Mult> get_mults() const;

    [[nodiscard]] Group product(const Group &other) const;

    [[nodiscard]] Group power(int p) const;
};
