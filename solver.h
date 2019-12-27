#pragma once

#include "groups.h"
#include <vector>

namespace tc {
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

    struct RelTable {
        int gens[2]{};
        int mult;
        std::vector<int *> lst_ptr;
        std::vector<int> gen;

        explicit RelTable(Mult m);

        int add_row();
    };

    Cosets solve(const Group &g, const std::vector<int> &sub_gens = {});
}